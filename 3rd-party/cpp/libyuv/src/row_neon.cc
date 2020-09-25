/*
 *  Copyright 2011 The LibYuv Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS. All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "libyuv/row.h"

#include <stdio.h>

#ifdef __cplusplus
namespace libyuv {
	extern "C" {
#endif

// This module is for GCC Neon
#if !defined(LIBYUV_DISABLE_NEON) && defined(__ARM_NEON__) && \
    !defined(__aarch64__)

// Read 8 Y and 4 VU from NV21
#define READNV21                                                               \
  "vld1.8     {d0}, [%0]!                    \n"                               \
  "vld1.8     {d2}, [%1]!                    \n"                               \
  "vmov.u8    d3, d2                         \n" /* split odd/even uv apart */ \
  "vuzp.u8    d3, d2                         \n"                               \
  "vtrn.u32   d2, d3                         \n"

#define YUVTORGB_SETUP                             \
  "vld1.8     {d24}, [%[kUVToRB]]            \n"   \
  "vld1.8     {d25}, [%[kUVToG]]             \n"   \
  "vld1.16    {d26[], d27[]}, [%[kUVBiasBGR]]! \n" \
  "vld1.16    {d8[], d9[]}, [%[kUVBiasBGR]]!   \n" \
  "vld1.16    {d28[], d29[]}, [%[kUVBiasBGR]]  \n" \
  "vld1.32    {d30[], d31[]}, [%[kYToRgb]]     \n"

#define YUVTORGB                                                              \
  "vmull.u8   q8, d2, d24                    \n" /* u/v B/R component      */ \
  "vmull.u8   q9, d2, d25                    \n" /* u/v G component        */ \
  "vmovl.u8   q0, d0                         \n" /* Y                      */ \
  "vmovl.s16  q10, d1                        \n"                              \
  "vmovl.s16  q0, d0                         \n"                              \
  "vmul.s32   q10, q10, q15                  \n"                              \
  "vmul.s32   q0, q0, q15                    \n"                              \
  "vqshrun.s32 d0, q0, #16                   \n"                              \
  "vqshrun.s32 d1, q10, #16                  \n" /* Y                      */ \
  "vadd.s16   d18, d19                       \n"                              \
  "vshll.u16  q1, d16, #16                   \n" /* Replicate u * UB       */ \
  "vshll.u16  q10, d17, #16                  \n" /* Replicate v * VR       */ \
  "vshll.u16  q3, d18, #16                   \n" /* Replicate (v*VG + u*UG)*/ \
  "vaddw.u16  q1, q1, d16                    \n"                              \
  "vaddw.u16  q10, q10, d17                  \n"                              \
  "vaddw.u16  q3, q3, d18                    \n"                              \
  "vqadd.s16  q8, q0, q13                    \n" /* B */                      \
  "vqadd.s16  q9, q0, q14                    \n" /* R */                      \
  "vqadd.s16  q0, q0, q4                     \n" /* G */                      \
  "vqadd.s16  q8, q8, q1                     \n" /* B */                      \
  "vqadd.s16  q9, q9, q10                    \n" /* R */                      \
  "vqsub.s16  q0, q0, q3                     \n" /* G */                      \
  "vqshrun.s16 d20, q8, #6                   \n" /* B */                      \
  "vqshrun.s16 d22, q9, #6                   \n" /* R */                      \
  "vqshrun.s16 d21, q0, #6                   \n" /* G */

void NV21ToRGB24Row_NEON(const uint8_t* src_y,
                         const uint8_t* src_vu,
                         uint8_t* dst_rgb24,
                         const struct YuvConstants* yuvconstants,
                         int width) {
  asm volatile(

      YUVTORGB_SETUP

      "1:                                        \n"

      READNV21 YUVTORGB
      "subs       %3, %3, #8                     \n"
      "vst3.8     {d20, d21, d22}, [%2]!         \n"
      "bgt        1b                             \n"
      : "+r"(src_y),      // %0
        "+r"(src_vu),     // %1
        "+r"(dst_rgb24),  // %2
        "+r"(width)       // %3
      : [kUVToRB] "r"(&yuvconstants->kUVToRB),
        [kUVToG] "r"(&yuvconstants->kUVToG),
        [kUVBiasBGR] "r"(&yuvconstants->kUVBiasBGR),
        [kYToRgb] "r"(&yuvconstants->kYToRgb)
      : "cc", "memory", "q0", "q1", "q2", "q3", "q4", "q8", "q9", "q10", "q11",
        "q12", "q13", "q14", "q15");
}



#endif

#ifdef __cplusplus
	}  // extern "C"
}  // namespace libyuv
#endif