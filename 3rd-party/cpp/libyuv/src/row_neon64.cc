/*
 *  Copyright 2014 The LibYuv Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS. All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "libyuv/row.h"

#ifdef __cplusplus
namespace libyuv {
	extern "C" {
#endif

// This module is for GCC Neon armv8 64 bit.
#if !defined(LIBYUV_DISABLE_NEON) && defined(__aarch64__)
// Read 8 Y and 4 VU from NV21
#define READNV21                                 \
  "ld1        {v0.8b}, [%0], #8              \n" \
  "ld1        {v2.8b}, [%1], #8              \n" \
  "uzp1       v3.8b, v2.8b, v2.8b            \n" \
  "uzp2       v1.8b, v2.8b, v2.8b            \n" \
  "ins        v1.s[1], v3.s[0]               \n"

#define YUVTORGB_SETUP                           \
  "ld1r       {v24.8h}, [%[kUVBiasBGR]], #2  \n" \
  "ld1r       {v25.8h}, [%[kUVBiasBGR]], #2  \n" \
  "ld1r       {v26.8h}, [%[kUVBiasBGR]]      \n" \
  "ld1r       {v31.4s}, [%[kYToRgb]]         \n" \
  "ld2        {v27.8h, v28.8h}, [%[kUVToRB]] \n" \
  "ld2        {v29.8h, v30.8h}, [%[kUVToG]]  \n"

#define YUVTORGB(vR, vG, vB)                                        \
  "uxtl       v0.8h, v0.8b                   \n" /* Extract Y    */ \
  "shll       v2.8h, v1.8b, #8               \n" /* Replicate UV */ \
  "ushll2     v3.4s, v0.8h, #0               \n" /* Y */            \
  "ushll      v0.4s, v0.4h, #0               \n"                    \
  "mul        v3.4s, v3.4s, v31.4s           \n"                    \
  "mul        v0.4s, v0.4s, v31.4s           \n"                    \
  "sqshrun    v0.4h, v0.4s, #16              \n"                    \
  "sqshrun2   v0.8h, v3.4s, #16              \n" /* Y */            \
  "uaddw      v1.8h, v2.8h, v1.8b            \n" /* Replicate UV */ \
  "mov        v2.d[0], v1.d[1]               \n" /* Extract V */    \
  "uxtl       v2.8h, v2.8b                   \n"                    \
  "uxtl       v1.8h, v1.8b                   \n" /* Extract U */    \
  "mul        v3.8h, v1.8h, v27.8h           \n"                    \
  "mul        v5.8h, v1.8h, v29.8h           \n"                    \
  "mul        v6.8h, v2.8h, v30.8h           \n"                    \
  "mul        v7.8h, v2.8h, v28.8h           \n"                    \
  "sqadd      v6.8h, v6.8h, v5.8h            \n"                    \
  "sqadd      " #vB                                                 \
  ".8h, v24.8h, v0.8h      \n" /* B */                              \
  "sqadd      " #vG                                                 \
  ".8h, v25.8h, v0.8h      \n" /* G */                              \
  "sqadd      " #vR                                                 \
  ".8h, v26.8h, v0.8h      \n" /* R */                              \
  "sqadd      " #vB ".8h, " #vB                                     \
  ".8h, v3.8h  \n" /* B */                                          \
  "sqsub      " #vG ".8h, " #vG                                     \
  ".8h, v6.8h  \n" /* G */                                          \
  "sqadd      " #vR ".8h, " #vR                                     \
  ".8h, v7.8h  \n" /* R */                                          \
  "sqshrun    " #vB ".8b, " #vB                                     \
  ".8h, #6     \n" /* B */                                          \
  "sqshrun    " #vG ".8b, " #vG                                     \
  ".8h, #6     \n"                               /* G */            \
  "sqshrun    " #vR ".8b, " #vR ".8h, #6     \n" /* R */

void NV21ToRGB24Row_NEON(const uint8_t* src_y,
                         const uint8_t* src_vu,
                         uint8_t* dst_rgb24,
                         const struct YuvConstants* yuvconstants,
                         int width) {
  asm volatile (
    YUVTORGB_SETUP
  "1:                                          \n"
    READNV21
    YUVTORGB(v22, v21, v20)
    "subs       %w3, %w3, #8                   \n"
    "st3        {v20.8b,v21.8b,v22.8b}, [%2], #24     \n"
    "b.gt       1b                             \n"
    : "+r"(src_y),     // %0
      "+r"(src_vu),    // %1
      "+r"(dst_rgb24),  // %2
      "+r"(width)      // %3
    : [kUVToRB]"r"(&yuvconstants->kUVToRB),
      [kUVToG]"r"(&yuvconstants->kUVToG),
      [kUVBiasBGR]"r"(&yuvconstants->kUVBiasBGR),
      [kYToRgb]"r"(&yuvconstants->kYToRgb)
    : "cc", "memory", "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v20",
      "v21", "v22", "v23", "v24", "v25", "v26", "v27", "v28", "v29", "v30"
  );
}

#endif

#ifdef __cplusplus
	}  // extern "C"
}  // namespace libyuv
#endif