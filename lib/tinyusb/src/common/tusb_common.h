/**************************************************************************/
/*!
    @file     tusb_common.h
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

/** \ingroup Group_Common
 *  \defgroup Group_CommonH common.h
 *  @{ */

#ifndef _TUSB_COMMON_H_
#define _TUSB_COMMON_H_

#ifdef __cplusplus
 extern "C" {
#endif

 //--------------------------------------------------------------------+
// Macros Helper
//--------------------------------------------------------------------+
#define TU_ARRAY_SZIE(_arr)      ( sizeof(_arr) / sizeof(_arr[0]) )
#define TU_MIN(_x, _y)           ( (_x) < (_y) ) ? (_x) : (_y) )
#define TU_MAX(_x, _y)           ( (_x) > (_y) ) ? (_x) : (_y) )

#define U16_HIGH_U8(u16) ((uint8_t) (((u16) >> 8) & 0x00ff))
#define U16_LOW_U8(u16)  ((uint8_t) ((u16)       & 0x00ff))
#define U16_TO_U8S_BE(u16)  U16_HIGH_U8(u16), U16_LOW_U8(u16)
#define U16_TO_U8S_LE(u16)  U16_LOW_U8(u16), U16_HIGH_U8(u16)

#define U32_B1_U8(u32) ((uint8_t) (((u32) >> 24) & 0x000000ff)) // MSB
#define U32_B2_U8(u32) ((uint8_t) (((u32) >> 16) & 0x000000ff))
#define U32_B3_U8(u32) ((uint8_t) (((u32) >>  8) & 0x000000ff))
#define U32_B4_U8(u32) ((uint8_t) ((u32)        & 0x000000ff)) // LSB

#define U32_TO_U8S_BE(u32) U32_B1_U8(u32), U32_B2_U8(u32), U32_B3_U8(u32), U32_B4_U8(u32)
#define U32_TO_U8S_LE(u32) U32_B4_U8(u32), U32_B3_U8(u32), U32_B2_U8(u32), U32_B1_U8(u32)

//------------- Bit -------------//
#define TU_BIT(n) (1U << (n))                                   ///< n-th Bit
#define TU_BIT_SET(x, n) ( (x) | TU_BIT(n) )                    ///< set n-th bit of x to 1
#define TU_BIT_CLEAR(x, n) ( (x) & (~TU_BIT(n)) )               ///< clear n-th bit of x
#define TU_BIT_TEST(x, n) ( ((x) & TU_BIT(n)) ? true : false )  ///< check if n-th bit of x is 1

//------------- Endian Conversion -------------//
#define ENDIAN_BE(u32) \
    (uint32_t) ( (((u32) & 0xFF) << 24) | (((u32) & 0xFF00) << 8) | (((u32) >> 8) & 0xFF00) | (((u32) >> 24) & 0xFF) )

#define ENDIAN_BE16(le16) ((uint16_t) ((U16_LOW_U8(le16) << 8) | U16_HIGH_U8(le16)) )

//------------- Binary constant -------------//
#if defined(__GNUC__) && !defined(__CC_ARM)

#define TU_BIN8(x)               ((uint8_t)  (0b##x))
#define TU_BIN16(b1, b2)         ((uint16_t) (0b##b1##b2))
#define TU_BIN32(b1, b2, b3, b4) ((uint32_t) (0b##b1##b2##b3##b4))

#else

//  internal macro of B8, B16, B32
#define _B8__(x) (((x&0x0000000FUL)?1:0) \
                +((x&0x000000F0UL)?2:0) \
                +((x&0x00000F00UL)?4:0) \
                +((x&0x0000F000UL)?8:0) \
                +((x&0x000F0000UL)?16:0) \
                +((x&0x00F00000UL)?32:0) \
                +((x&0x0F000000UL)?64:0) \
                +((x&0xF0000000UL)?128:0))

#define TU_BIN8(d) ((uint8_t) _B8__(0x##d##UL))
#define TU_BIN16(dmsb,dlsb) (((uint16_t)TU_BIN8(dmsb)<<8) + TU_BIN8(dlsb))
#define TU_BIN32(dmsb,db2,db3,dlsb) \
            (((uint32_t)TU_BIN8(dmsb)<<24) \
            + ((uint32_t)TU_BIN8(db2)<<16) \
            + ((uint32_t)TU_BIN8(db3)<<8) \
            + TU_BIN8(dlsb))
#endif

// for declaration of reserved field, make use of _TU_COUNTER_
#define TU_RESERVED   XSTRING_CONCAT_(reserved, _TU_COUNTER_)

//--------------------------------------------------------------------+
// INCLUDES
//--------------------------------------------------------------------+

// Standard Headers
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

// Tinyusb Common Headers
#include "tusb_option.h"
#include "tusb_compiler.h"
#include "tusb_verify.h"
//#include "binary.h"
#include "tusb_error.h"
#include "tusb_timeout.h"
#include "tusb_types.h"

//--------------------------------------------------------------------+
// INLINE FUNCTION
//--------------------------------------------------------------------+
#ifndef __n2be_16 // TODO clean up
#define __n2be_16(u16)  ((uint16_t) ((U16_LOW_U8(u16) << 8) | U16_HIGH_U8(u16)) )
#define __be2n_16(u16)  __n2be_16(u16)
#endif

#define tu_memclr(buffer, size)  memset((buffer), 0, (size))
#define tu_varclr(_var)          tu_memclr(_var, sizeof(*(_var)))

static inline bool tu_mem_test_zero (void const* buffer, uint32_t size)
{
  uint8_t const* p_mem = (uint8_t const*) buffer;
  for(uint32_t i=0; i<size; i++) if (p_mem[i] != 0)  return false;
  return true;
}


//------------- Conversion -------------//
static inline uint32_t tu_u32_from_u8(uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4)
{
  return ( ((uint32_t) b1) << 24) + ( ((uint32_t) b2) << 16) + ( ((uint32_t) b3) << 8) + b4;
}

static inline uint8_t tu_u16_high(uint16_t u16)
{
  return (uint8_t) ( ((uint16_t) (u16 >> 8)) & 0x00ff);
}

static inline uint8_t tu_u16_low(uint16_t u16)
{
  return (uint8_t) (u16 & 0x00ff);
}

static inline uint16_t tu_u16_le2be(uint16_t u16)
{
  return ((uint16_t)(tu_u16_low(u16) << 8)) | tu_u16_high(u16);
}

// Min
static inline uint8_t  tu_min8(uint8_t x, uint8_t y) { return (x < y) ? x : y; }
static inline uint16_t tu_min16(uint16_t x, uint16_t y) { return (x < y) ? x : y; }
static inline uint32_t tu_min32(uint32_t x, uint32_t y) { return (x < y) ? x : y; }

// Max
static inline uint8_t  tu_max8(uint8_t x, uint8_t y) { return (x > y) ? x : y; }
static inline uint16_t tu_max16(uint16_t x, uint16_t y) { return (x > y) ? x : y; }
static inline uint32_t tu_max32(uint32_t x, uint32_t y) { return (x > y) ? x : y; }

// Align
static inline uint32_t tu_align32 (uint32_t value) { return (value & 0xFFFFFFE0UL); }
static inline uint32_t tu_align16 (uint32_t value) { return (value & 0xFFFFFFF0UL); }
static inline uint32_t tu_align_n (uint32_t alignment, uint32_t value) { return value & ((uint32_t) ~(alignment-1)); }
static inline uint32_t tu_align4k (uint32_t value) { return (value & 0xFFFFF000UL); }

static inline uint32_t tu_offset4k(uint32_t value) { return (value & 0xFFFUL); }

//------------- Mathematics -------------//
static inline uint32_t tu_abs(int32_t value) { return (value < 0) ? (-value) : value; }

/// inclusive range checking
static inline bool tu_within(uint32_t lower, uint32_t value, uint32_t upper)
{
  return (lower <= value) && (value <= upper);
}

// log2 of a value is its MSB's position
// TODO use clz
static inline uint8_t tu_log2(uint32_t value)
{
  uint8_t result = 0;

  while (value >>= 1)
  {
    result++;
  }
  return result;
}

// Bit
static inline uint32_t tu_bit_set(uint32_t value, uint8_t n) { return value | TU_BIT(n); }
static inline uint32_t tu_bit_clear(uint32_t value, uint8_t n) { return value & (~TU_BIT(n)); }
static inline bool tu_bit_test(uint32_t value, uint8_t n) { return (value & TU_BIT(n)) ? true : false; }

/*------------------------------------------------------------------*/
/* Count number of arguments of __VA_ARGS__
 * - reference https://groups.google.com/forum/#!topic/comp.std.c/d-6Mj5Lko_s
 * - _GET_NTH_ARG() takes args >= N (64) but only expand to Nth one (64th)
 * - _RSEQ_N() is reverse sequential to N to add padding to have
 * Nth position is the same as the number of arguments
 * - ##__VA_ARGS__ is used to deal with 0 paramerter (swallows comma)
 *------------------------------------------------------------------*/
#ifndef VA_ARGS_NUM_

#define VA_ARGS_NUM_(...) 	 NARG_(_0, ##__VA_ARGS__,_RSEQ_N())
#define NARG_(...)        _GET_NTH_ARG(__VA_ARGS__)
#define _GET_NTH_ARG( \
          _1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
         _11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
         _21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
         _31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
         _41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
         _51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
         _61,_62,_63,N,...) N
#define _RSEQ_N() \
         62,61,60,                      \
         59,58,57,56,55,54,53,52,51,50, \
         49,48,47,46,45,44,43,42,41,40, \
         39,38,37,36,35,34,33,32,31,30, \
         29,28,27,26,25,24,23,22,21,20, \
         19,18,17,16,15,14,13,12,11,10, \
         9,8,7,6,5,4,3,2,1,0
#endif

#ifdef __cplusplus
 }
#endif

#endif /* _TUSB_COMMON_H_ */

/**  @} */
