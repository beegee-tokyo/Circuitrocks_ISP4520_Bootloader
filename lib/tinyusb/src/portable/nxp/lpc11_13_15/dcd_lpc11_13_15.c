/**************************************************************************/
/*!
    @file     dcd_lpc11_13_15.c
    @author   hathach (tinyusb.org)

    @section LICENSE

    Software License Agreement (BSD License)

    Copyright (c) 2013, hathach (tinyusb.org)
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    1. Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    3. Neither the name of the copyright holders nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    This file is part of the tinyusb stack.
*/
/**************************************************************************/

#include "tusb_option.h"

#if TUSB_OPT_DEVICE_ENABLED && (CFG_TUSB_MCU == OPT_MCU_LPC11UXX || CFG_TUSB_MCU == OPT_MCU_LPC13XX)

#include "chip.h"
#include "device/dcd.h"
#include "dcd_lpc11_13_15.h"

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF
//--------------------------------------------------------------------+

// Number of endpoints
#define EP_COUNT 10

// only SRAM1 & USB RAM can be used for transfer
#define SRAM_REGION   0x20000000

/* Although device controller are the same. DMA of
 * - LPC11u can only transfer up to nbytes = 64
 * - LPC13 can transfer nbytes = 1023 (maximum)
 * - LPC15 can ???
 */
enum {
  DMA_NBYTES_MAX = (CFG_TUSB_MCU == OPT_MCU_LPC11UXX ? 64 : 1023)
};

enum {
  INT_SOF_MASK           = TU_BIT(30),
  INT_DEVICE_STATUS_MASK = TU_BIT(31)
};

enum {
  CMDSTAT_DEVICE_ADDR_MASK    = TU_BIT(7 )-1,
  CMDSTAT_DEVICE_ENABLE_MASK  = TU_BIT(7 ),
  CMDSTAT_SETUP_RECEIVED_MASK = TU_BIT(8 ),
  CMDSTAT_DEVICE_CONNECT_MASK = TU_BIT(16), ///< reflect the softconnect only, does not reflect the actual attached state
  CMDSTAT_DEVICE_SUSPEND_MASK = TU_BIT(17),
  CMDSTAT_CONNECT_CHANGE_MASK = TU_BIT(24),
  CMDSTAT_SUSPEND_CHANGE_MASK = TU_BIT(25),
  CMDSTAT_RESET_CHANGE_MASK   = TU_BIT(26),
  CMDSTAT_VBUS_DEBOUNCED_MASK = TU_BIT(28),
};

typedef struct ATTR_PACKED
{
  // Bits 21:6 (aligned 64) used in conjunction with bit 31:22 of DATABUFSTART
  volatile uint16_t buffer_offset;

  volatile uint16_t nbytes : 10 ;
  uint16_t is_iso          : 1  ;
  uint16_t toggle_mode     : 1  ;
  uint16_t toggle_reset    : 1  ;
  uint16_t stall           : 1  ;
  uint16_t disable         : 1  ;
  volatile uint16_t active : 1  ;
}ep_cmd_sts_t;

TU_VERIFY_STATIC( sizeof(ep_cmd_sts_t) == 4, "size is not correct" );

typedef struct
{
  uint16_t total_bytes;
  uint16_t xferred_bytes;

  uint16_t nbytes;
}xfer_dma_t;

// NOTE data will be transferred as soon as dcd get request by dcd_pipe(_queue)_xfer using double buffering.
// current_td is used to keep track of number of remaining & xferred bytes of the current request.
typedef struct
{
  // 256 byte aligned, 2 for double buffer (not used)
  // Each cmd_sts can only transfer up to DMA_NBYTES_MAX bytes each
  ep_cmd_sts_t ep[EP_COUNT][2];

  xfer_dma_t dma[EP_COUNT];

  ATTR_ALIGNED(64) uint8_t setup_packet[8];
}dcd_data_t;

//--------------------------------------------------------------------+
// INTERNAL OBJECT & FUNCTION DECLARATION
//--------------------------------------------------------------------+

// EP list must be 256-byte aligned
CFG_TUSB_MEM_SECTION ATTR_ALIGNED(256) static dcd_data_t _dcd;

static inline uint16_t get_buf_offset(void const * buffer)
{
  uint32_t addr = (uint32_t) buffer;
  TU_ASSERT( (addr & 0x3f) == 0, 0 );
  return ( (addr >> 6) & 0xFFFFUL ) ;
}

static inline uint8_t ep_addr2id(uint8_t endpoint_addr)
{
  return 2*(endpoint_addr & 0x0F) + ((endpoint_addr & TUSB_DIR_IN_MASK) ? 1 : 0);
}

//--------------------------------------------------------------------+
// CONTROLLER API
//--------------------------------------------------------------------+
void dcd_int_enable(uint8_t rhport)
{
  (void) rhport;
  NVIC_EnableIRQ(USB0_IRQn);
}

void dcd_int_disable(uint8_t rhport)
{
  (void) rhport;
  NVIC_DisableIRQ(USB0_IRQn);
}

void dcd_set_config(uint8_t rhport, uint8_t config_num)
{
  (void) rhport;
  (void) config_num;
}

void dcd_set_address(uint8_t rhport, uint8_t dev_addr)
{
  (void) rhport;

  LPC_USB->DEVCMDSTAT &= ~CMDSTAT_DEVICE_ADDR_MASK;
  LPC_USB->DEVCMDSTAT |= dev_addr;
}

uint32_t dcd_get_frame_number(uint8_t rhport)
{
  (void) rhport;

  return LPC_USB->INFO & (TU_BIT(11) - 1);
}

bool dcd_init(uint8_t rhport)
{
  (void) rhport;

  LPC_USB->EPLISTSTART  = (uint32_t) _dcd.ep;
  LPC_USB->DATABUFSTART = SRAM_REGION;

  LPC_USB->INTSTAT      = LPC_USB->INTSTAT; // clear all pending interrupt
  LPC_USB->INTEN        = INT_DEVICE_STATUS_MASK;
  LPC_USB->DEVCMDSTAT  |= CMDSTAT_DEVICE_ENABLE_MASK | CMDSTAT_DEVICE_CONNECT_MASK |
                          CMDSTAT_RESET_CHANGE_MASK | CMDSTAT_CONNECT_CHANGE_MASK | CMDSTAT_SUSPEND_CHANGE_MASK;

  NVIC_EnableIRQ(USB0_IRQn);

  return true;
}

//--------------------------------------------------------------------+
// DCD Endpoint Port
//--------------------------------------------------------------------+
void dcd_edpt_stall(uint8_t rhport, uint8_t ep_addr)
{
  (void) rhport;

  if ( tu_edpt_number(ep_addr) == 0 )
  {
    // TODO cannot able to STALL Control OUT endpoint !!!!! FIXME try some walk-around
    _dcd.ep[0][0].stall = _dcd.ep[1][0].stall = 1;
  }
  else
  {
    uint8_t const ep_id = ep_addr2id(ep_addr);
    _dcd.ep[ep_id][0].stall = 1;
  }
}

bool dcd_edpt_stalled(uint8_t rhport, uint8_t ep_addr)
{
  (void) rhport;

  uint8_t const ep_id = ep_addr2id(ep_addr);
  return _dcd.ep[ep_id][0].stall;
}

void dcd_edpt_clear_stall(uint8_t rhport, uint8_t ep_addr)
{
  (void) rhport;

  uint8_t const ep_id = ep_addr2id(ep_addr);

  _dcd.ep[ep_id][0].stall        = 0;
  _dcd.ep[ep_id][0].toggle_reset = 1;
  _dcd.ep[ep_id][0].toggle_mode  = 0;
}

bool dcd_edpt_open(uint8_t rhport, tusb_desc_endpoint_t const * p_endpoint_desc)
{
  (void) rhport;

  // TODO not support ISO yet
  if (p_endpoint_desc->bmAttributes.xfer == TUSB_XFER_ISOCHRONOUS) return false;

  //------------- Prepare Queue Head -------------//
  uint8_t ep_id = ep_addr2id(p_endpoint_desc->bEndpointAddress);

  // Check if endpoint is available
  TU_ASSERT( _dcd.ep[ep_id][0].disable && _dcd.ep[ep_id][1].disable );

  tu_memclr(_dcd.ep[ep_id], 2*sizeof(ep_cmd_sts_t));
  _dcd.ep[ep_id][0].is_iso = (p_endpoint_desc->bmAttributes.xfer == TUSB_XFER_ISOCHRONOUS);

  // Enable EP interrupt
  LPC_USB->INTEN |= TU_BIT(ep_id);

  return true;
}

bool dcd_edpt_busy(uint8_t rhport, uint8_t ep_addr)
{
  (void) rhport;

  uint8_t const ep_id = ep_addr2id(ep_addr);
  return _dcd.ep[ep_id][0].active;
}

static void prepare_ep_xfer(uint8_t ep_id, uint16_t buf_offset, uint16_t total_bytes)
{
  uint16_t const nbytes = tu_min16(total_bytes, DMA_NBYTES_MAX);

  _dcd.dma[ep_id].nbytes = nbytes;

  _dcd.ep[ep_id][0].buffer_offset = buf_offset;
  _dcd.ep[ep_id][0].nbytes        = nbytes;
  _dcd.ep[ep_id][0].active        = 1;
}

bool dcd_edpt_xfer(uint8_t rhport, uint8_t ep_addr, uint8_t* buffer, uint16_t total_bytes)
{
  (void) rhport;

  uint8_t const ep_id = ep_addr2id(ep_addr);

  tu_varclr(&_dcd.dma[ep_id]);
  _dcd.dma[ep_id].total_bytes = total_bytes;

  prepare_ep_xfer(ep_id, get_buf_offset(buffer), total_bytes);

	return true;
}

//--------------------------------------------------------------------+
// IRQ
//--------------------------------------------------------------------+
static void bus_reset(void)
{
  tu_memclr(&_dcd, sizeof(dcd_data_t));

  // disable all non-control endpoints on bus reset
  for(uint8_t ep_id = 2; ep_id < EP_COUNT; ep_id++)
  {
    _dcd.ep[ep_id][0].disable = _dcd.ep[ep_id][1].disable = 1;
  }

  _dcd.ep[0][1].buffer_offset = get_buf_offset(_dcd.setup_packet);

  LPC_USB->EPINUSE      = 0;
  LPC_USB->EPBUFCFG     = 0;
  LPC_USB->EPSKIP       = 0xFFFFFFFF;

  LPC_USB->INTSTAT      = LPC_USB->INTSTAT; // clear all pending interrupt
  LPC_USB->DEVCMDSTAT  |= CMDSTAT_SETUP_RECEIVED_MASK; // clear setup received interrupt
  LPC_USB->INTEN        = INT_DEVICE_STATUS_MASK | TU_BIT(0) | TU_BIT(1); // enable device status & control endpoints
}

static void process_xfer_isr(uint32_t int_status)
{
  for(uint8_t ep_id = 0; ep_id < EP_COUNT; ep_id++ )
  {
    if ( TU_BIT_TEST(int_status, ep_id) )
    {
      ep_cmd_sts_t * ep_cs = &_dcd.ep[ep_id][0];
      xfer_dma_t* xfer_dma = &_dcd.dma[ep_id];

      xfer_dma->xferred_bytes += xfer_dma->nbytes - ep_cs->nbytes;

      if ( (ep_cs->nbytes == 0) && (xfer_dma->total_bytes > xfer_dma->xferred_bytes) )
      {
        // There is more data to transfer
        // buff_offset has been already increased by hw to correct value for next transfer
        prepare_ep_xfer(ep_id, ep_cs->buffer_offset, xfer_dma->total_bytes - xfer_dma->xferred_bytes);
      }
      else
      {
        xfer_dma->total_bytes = xfer_dma->xferred_bytes;

        uint8_t const ep_addr = (ep_id / 2) | ((ep_id & 0x01) ? TUSB_DIR_IN_MASK : 0);

        // TODO no way determine if the transfer is failed or not
        dcd_event_xfer_complete(0, ep_addr, xfer_dma->xferred_bytes, XFER_RESULT_SUCCESS, true);
      }
    }
  }
}

void USB_IRQHandler(void)
{
  uint32_t const dev_cmd_stat = LPC_USB->DEVCMDSTAT;

  uint32_t int_status = LPC_USB->INTSTAT & LPC_USB->INTEN;
  LPC_USB->INTSTAT = int_status; // Acknowledge handled interrupt

  if (int_status == 0) return;

  //------------- Device Status -------------//
  if ( int_status & INT_DEVICE_STATUS_MASK )
  {
    LPC_USB->DEVCMDSTAT |= CMDSTAT_RESET_CHANGE_MASK | CMDSTAT_CONNECT_CHANGE_MASK | CMDSTAT_SUSPEND_CHANGE_MASK;
    if ( dev_cmd_stat & CMDSTAT_RESET_CHANGE_MASK) // bus reset
    {
      bus_reset();
      dcd_event_bus_signal(0, DCD_EVENT_BUS_RESET, true);
    }

    if (dev_cmd_stat & CMDSTAT_CONNECT_CHANGE_MASK)
    {
      // device disconnect
      if (dev_cmd_stat & CMDSTAT_DEVICE_ADDR_MASK)
      {
        // debouncing as this can be set when device is powering
        dcd_event_bus_signal(0, DCD_EVENT_UNPLUGGED, true);
      }
    }

    // TODO support suspend & resume
    if (dev_cmd_stat & CMDSTAT_SUSPEND_CHANGE_MASK)
    {
      if (dev_cmd_stat & CMDSTAT_DEVICE_SUSPEND_MASK)
      { // suspend signal, bus idle for more than 3ms
        // Note: Host may delay more than 3 ms before and/or after bus reset before doing enumeration.
        if (dev_cmd_stat & CMDSTAT_DEVICE_ADDR_MASK)
        {
          dcd_event_bus_signal(0, DCD_EVENT_SUSPENDED, true);
        }
      }
    }
//        else
//      { // resume signal
//    dcd_event_bus_signal(0, DCD_EVENT_RESUME, true);
//      }
//    }
  }

  // Setup Receive
  if ( TU_BIT_TEST(int_status, 0) && (dev_cmd_stat & CMDSTAT_SETUP_RECEIVED_MASK) )
  {
    // Follow UM flowchart to clear Active & Stall on both Control IN/OUT endpoints
    _dcd.ep[0][0].active = _dcd.ep[1][0].active = 0;
    _dcd.ep[0][0].stall = _dcd.ep[1][0].stall = 0;

    LPC_USB->DEVCMDSTAT |= CMDSTAT_SETUP_RECEIVED_MASK;

    dcd_event_setup_received(0, _dcd.setup_packet, true);

    // keep waiting for next setup
    _dcd.ep[0][1].buffer_offset = get_buf_offset(_dcd.setup_packet);

    // clear bit0
    int_status = TU_BIT_CLEAR(int_status, 0);
  }

  // Endpoint transfer complete interrupt
  process_xfer_isr(int_status);
}

#endif

