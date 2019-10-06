/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Ha Thach for Adafruit Industries
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#ifdef __cplusplus
 extern "C" {
#endif

//--------------------------------------------------------------------+
// COMMON CONFIGURATION
//--------------------------------------------------------------------+
#define CFG_TUSB_MCU                OPT_MCU_NRF5X
#define CFG_TUSB_RHPORT0_MODE       OPT_MODE_DEVICE

#ifndef CFG_TUSB_DEBUG
#define CFG_TUSB_DEBUG              0
#endif

/*------------- RTOS -------------*/
#define CFG_TUSB_OS                 OPT_OS_NONE
//#define CFG_TUD_TASK_PRIO         0
//#define CFG_TUD_TASK_QUEUE_SZ     16
//#define CFG_TUD_TASK_STACK_SZ     150


//--------------------------------------------------------------------+
// DEVICE CONFIGURATION
// Note: TUD Stand for Tiny Usb Device
//--------------------------------------------------------------------+

/*------------- Core -------------*/
#define CFG_TUD_DESC_AUTO           0
#define CFG_TUD_ENDOINT0_SIZE       64

//------------- Class enabled -------------//
#define CFG_TUD_CDC                 1
#define CFG_TUD_MSC                 1
#define CFG_TUD_HID_KEYBOARD        0
#define CFG_TUD_HID_MOUSE           0
#define CFG_TUD_HID_GENERIC         0 // not supported yet
#define CFG_TUD_CUSTOM_CLASS        0


/*------------------------------------------------------------------*/
/* CLASS DRIVER
 *------------------------------------------------------------------*/

// FIFO size of CDC TX and RX
#define CFG_TUD_CDC_RX_BUFSIZE      1024
#define CFG_TUD_CDC_TX_BUFSIZE      1024

/* TX is sent automatically on every Start of Frame event ~ 1ms.
 * If not enabled, application must call tud_cdc_flush() periodically
 * Note: Enabled this could overflow device task, if it does, define
 * CFG_TUD_TASK_QUEUE_SZ with large value
 */
#define CFG_TUD_CDC_FLUSH_ON_SOF    0

// Number of supported Logical Unit Number
#define CFG_TUD_MSC_MAXLUN          1

// Buffer size for each read/write transfer, the more the better
#define CFG_TUD_MSC_BUFSIZE         (4*1024)

// Vendor name included in Inquiry response, max 8 bytes
#define CFG_TUD_MSC_VENDOR          "Adafruit"

// Product name included in Inquiry response, max 16 bytes
#define CFG_TUD_MSC_PRODUCT         "Feather nRF52840"

// Product revision string included in Inquiry response, max 4 bytes
#define CFG_TUD_MSC_PRODUCT_REV     "1.0"


//--------------------------------------------------------------------+
// USB RAM PLACEMENT
//--------------------------------------------------------------------+
#define CFG_TUSB_ATTR_USBRAM
#define CFG_TUSB_MEM_ALIGN          ATTR_ALIGNED(4)


#define BREAKPOINT_IGNORE_COUNT(n) \
  do {\
    static uint8_t ignore_count = 0;\
    ignore_count++;\
    if ( ignore_count > n ) verify_breakpoint();\
  }while(0)

#ifdef __cplusplus
 }
#endif

#endif /* _TUSB_CONFIG_H_ */
