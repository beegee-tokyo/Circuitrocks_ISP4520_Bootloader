/**************************************************************************/
/*!
    @file     hcd.h
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
    INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    INCLUDING NEGLIGENCE OR OTHERWISE ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	  This file is part of the tinyusb stack.
*/
/**************************************************************************/

/** \ingroup group_usbh
 * \defgroup Group_HCD Host Controller Driver (HCD)
 *  @{ */

#ifndef _TUSB_HCD_H_
#define _TUSB_HCD_H_

#include <common/tusb_common.h>

#ifdef __cplusplus
 extern "C" {
#endif

 //--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF
//--------------------------------------------------------------------+
typedef enum
{
  HCD_EVENT_DEVICE_ATTACH,
  HCD_EVENT_DEVICE_REMOVE,
  HCD_EVENT_XFER_COMPLETE,
} hcd_eventid_t;

typedef struct
{
  uint8_t rhport;
  uint8_t event_id;

  union
  {
    struct
    {
      uint8_t hub_addr;
      uint8_t hub_port;
    } attach, remove;

    struct
    {
      uint8_t ep_addr;
      uint8_t result;
      uint32_t len;
    } xfer_complete;
  };

} hcd_event_t;

#if TUSB_OPT_HOST_ENABLED
// Max number of endpoints per device
enum {
  HCD_MAX_ENDPOINT = CFG_TUSB_HOST_DEVICE_MAX*(CFG_TUH_HUB + CFG_TUH_HID_KEYBOARD + CFG_TUH_HID_MOUSE + CFG_TUSB_HOST_HID_GENERIC +
                     CFG_TUH_MSC*2 + CFG_TUH_CDC*3),

  HCD_MAX_XFER     = HCD_MAX_ENDPOINT*2,
};

//#define HCD_MAX_ENDPOINT 16
//#define HCD_MAX_XFER 16
#endif

//--------------------------------------------------------------------+
// HCD API
//--------------------------------------------------------------------+
bool hcd_init(void);
void hcd_int_enable (uint8_t rhport);
void hcd_int_disable(uint8_t rhport);

// PORT API
/// return the current connect status of roothub port
bool hcd_port_connect_status(uint8_t hostid);
void hcd_port_reset(uint8_t hostid);
tusb_speed_t hcd_port_speed_get(uint8_t hostid);

// HCD closes all opened endpoints belong to this device
void hcd_device_close(uint8_t rhport, uint8_t dev_addr);

//--------------------------------------------------------------------+
// Event function
//--------------------------------------------------------------------+
void hcd_event_handler(hcd_event_t const* event, bool in_isr);

// Helper to send device attach event
void hcd_event_device_attach(uint8_t rhport);

// Helper to send device removal event
void hcd_event_device_remove(uint8_t rhport);

// Helper to send USB transfer event
void hcd_event_xfer_complete(uint8_t dev_addr, uint8_t ep_addr, xfer_result_t event, uint32_t xferred_bytes);

//--------------------------------------------------------------------+
// Endpoints API
//--------------------------------------------------------------------+
bool hcd_setup_send(uint8_t rhport, uint8_t dev_addr, uint8_t const setup_packet[8]);
bool hcd_edpt_open(uint8_t rhport, uint8_t dev_addr, tusb_desc_endpoint_t const * ep_desc);

bool hcd_edpt_busy(uint8_t dev_addr, uint8_t ep_addr);
bool hcd_edpt_stalled(uint8_t dev_addr, uint8_t ep_addr);
bool hcd_edpt_clear_stall(uint8_t dev_addr, uint8_t ep_addr);

// TODO merge with pipe_xfer
bool hcd_edpt_xfer(uint8_t rhport, uint8_t dev_addr, uint8_t ep_addr, uint8_t * buffer, uint16_t buflen);

//--------------------------------------------------------------------+
// PIPE API
//--------------------------------------------------------------------+
// TODO control xfer should be used via usbh layer
bool hcd_pipe_queue_xfer(uint8_t dev_addr, uint8_t ep_addr, uint8_t buffer[], uint16_t total_bytes); // only queue, not transferring yet
bool hcd_pipe_xfer(uint8_t dev_addr, uint8_t ep_addr, uint8_t buffer[], uint16_t total_bytes, bool int_on_complete);

#if 0
tusb_error_t hcd_pipe_cancel();
#endif

#ifdef __cplusplus
 }
#endif

#endif /* _TUSB_HCD_H_ */

/// @}
