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

#ifdef __cplusplus
namespace libyuv {
	extern "C" {
#endif
// This module is for GCC x86 and x64.
#if !defined(LIBYUV_DISABLE_X86) && \
    (defined(__x86_64__) || (defined(__i386__) && !defined(_MSC_VER)))

#ifdef HAS_RGB24TOARGBROW_SSSE3
// Shuffle table for converting ARGB to RGB24.
static const uvec8 kShuffleMaskARGBToRGB24 = {
    0u, 1u, 2u, 4u, 5u, 6u, 8u, 9u, 10u, 12u, 13u, 14u, 128u, 128u, 128u, 128u};

// NV21 shuf 8 VU to 16 UV.
static const lvec8 kShuffleNV21 = {
    1, 0, 1, 0, 3, 2, 3, 2, 5, 4, 5, 4, 7, 6, 7, 6,
    1, 0, 1, 0, 3, 2, 3, 2, 5, 4, 5, 4, 7, 6, 7, 6,
};
#endif  // HAS_RGB24TOARGBROW_SSSE3

#ifdef HAS_RGB24TOARGBROW_SSSE3
void ARGBToRGB24Row_SSSE3(const uint8_t* src, uint8_t* dst, int width) {
  asm volatile(

      "movdqa    %3,%%xmm6                       \n"

      LABELALIGN
      "1:                                        \n"
      "movdqu    (%0),%%xmm0                     \n"
      "movdqu    0x10(%0),%%xmm1                 \n"
      "movdqu    0x20(%0),%%xmm2                 \n"
      "movdqu    0x30(%0),%%xmm3                 \n"
      "lea       0x40(%0),%0                     \n"
      "pshufb    %%xmm6,%%xmm0                   \n"
      "pshufb    %%xmm6,%%xmm1                   \n"
      "pshufb    %%xmm6,%%xmm2                   \n"
      "pshufb    %%xmm6,%%xmm3                   \n"
      "movdqa    %%xmm1,%%xmm4                   \n"
      "psrldq    $0x4,%%xmm1                     \n"
      "pslldq    $0xc,%%xmm4                     \n"
      "movdqa    %%xmm2,%%xmm5                   \n"
      "por       %%xmm4,%%xmm0                   \n"
      "pslldq    $0x8,%%xmm5                     \n"
      "movdqu    %%xmm0,(%1)                     \n"
      "por       %%xmm5,%%xmm1                   \n"
      "psrldq    $0x8,%%xmm2                     \n"
      "pslldq    $0x4,%%xmm3                     \n"
      "por       %%xmm3,%%xmm2                   \n"
      "movdqu    %%xmm1,0x10(%1)                 \n"
      "movdqu    %%xmm2,0x20(%1)                 \n"
      "lea       0x30(%1),%1                     \n"
      "sub       $0x10,%2                        \n"
      "jg        1b                              \n"
      : "+r"(src),                    // %0
        "+r"(dst),                    // %1
        "+r"(width)                   // %2
      : "m"(kShuffleMaskARGBToRGB24)  // %3
      : "memory", "cc", "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6");
}

#ifdef HAS_ARGBTORGB24ROW_AVX2
// vpermd for 12+12 to 24
static const lvec32 kPermdRGB24_AVX = { 0, 1, 2, 4, 5, 6, 3, 7 };

void ARGBToRGB24Row_AVX2(const uint8_t* src, uint8_t* dst, int width) {
    asm volatile(
        "vbroadcastf128 %3,%%ymm6                  \n"
        "vmovdqa    %4,%%ymm7                      \n"

        LABELALIGN
        "1:                                        \n"
        "vmovdqu    (%0),%%ymm0                    \n"
        "vmovdqu    0x20(%0),%%ymm1                \n"
        "vmovdqu    0x40(%0),%%ymm2                \n"
        "vmovdqu    0x60(%0),%%ymm3                \n"
        "lea        0x80(%0),%0                    \n"
        "vpshufb    %%ymm6,%%ymm0,%%ymm0           \n"  // xxx0yyy0
        "vpshufb    %%ymm6,%%ymm1,%%ymm1           \n"
        "vpshufb    %%ymm6,%%ymm2,%%ymm2           \n"
        "vpshufb    %%ymm6,%%ymm3,%%ymm3           \n"
        "vpermd     %%ymm0,%%ymm7,%%ymm0           \n"  // pack to 24 bytes
        "vpermd     %%ymm1,%%ymm7,%%ymm1           \n"
        "vpermd     %%ymm2,%%ymm7,%%ymm2           \n"
        "vpermd     %%ymm3,%%ymm7,%%ymm3           \n"
        "vpermq     $0x3f,%%ymm1,%%ymm4            \n"  // combine 24 + 8
        "vpor       %%ymm4,%%ymm0,%%ymm0           \n"
        "vmovdqu    %%ymm0,(%1)                    \n"
        "vpermq     $0xf9,%%ymm1,%%ymm1            \n"  // combine 16 + 16
        "vpermq     $0x4f,%%ymm2,%%ymm4            \n"
        "vpor       %%ymm4,%%ymm1,%%ymm1           \n"
        "vmovdqu    %%ymm1,0x20(%1)                \n"
        "vpermq     $0xfe,%%ymm2,%%ymm2            \n"  // combine 8 + 24
        "vpermq     $0x93,%%ymm3,%%ymm3            \n"
        "vpor       %%ymm3,%%ymm2,%%ymm2           \n"
        "vmovdqu    %%ymm2,0x40(%1)                \n"
        "lea        0x60(%1),%1                    \n"
        "sub        $0x20,%2                       \n"
        "jg         1b                             \n"
        "vzeroupper                                \n"
        : "+r"(src),                     // %0
        "+r"(dst),                     // %1
        "+r"(width)                    // %2
        : "m"(kShuffleMaskARGBToRGB24),  // %3
        "m"(kPermdRGB24_AVX)           // %4
        : "memory", "cc", "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6",
        "xmm7");
}
#endif

#endif  // HAS_RGB24TOARGBROW_SSSE3

#if defined(HAS_I422TOARGBROW_SSSE3) || defined(HAS_I422TOARGBROW_AVX2)

// Read 4 VU from NV21, upsample to 8 UV
#define READNV21                                                  \
  "movq       (%[vu_buf]),%%xmm0                              \n" \
  "lea        0x8(%[vu_buf]),%[vu_buf]                        \n" \
  "pshufb     %[kShuffleNV21], %%xmm0                         \n" \
  "movq       (%[y_buf]),%%xmm4                               \n" \
  "punpcklbw  %%xmm4,%%xmm4                                   \n" \
  "lea        0x8(%[y_buf]),%[y_buf]                          \n"

#if defined(__x86_64__)
#define YUVTORGB_SETUP(yuvconstants)                              \
  "movdqa     (%[yuvconstants]),%%xmm8                        \n" \
  "movdqa     32(%[yuvconstants]),%%xmm9                      \n" \
  "movdqa     64(%[yuvconstants]),%%xmm10                     \n" \
  "movdqa     96(%[yuvconstants]),%%xmm11                     \n" \
  "movdqa     128(%[yuvconstants]),%%xmm12                    \n" \
  "movdqa     160(%[yuvconstants]),%%xmm13                    \n" \
  "movdqa     192(%[yuvconstants]),%%xmm14                    \n"
// Convert 8 pixels: 8 UV and 8 Y
#define YUVTORGB16(yuvconstants)                                  \
  "movdqa     %%xmm0,%%xmm1                                   \n" \
  "movdqa     %%xmm0,%%xmm2                                   \n" \
  "movdqa     %%xmm0,%%xmm3                                   \n" \
  "movdqa     %%xmm11,%%xmm0                                  \n" \
  "pmaddubsw  %%xmm8,%%xmm1                                   \n" \
  "psubw      %%xmm1,%%xmm0                                   \n" \
  "movdqa     %%xmm12,%%xmm1                                  \n" \
  "pmaddubsw  %%xmm9,%%xmm2                                   \n" \
  "psubw      %%xmm2,%%xmm1                                   \n" \
  "movdqa     %%xmm13,%%xmm2                                  \n" \
  "pmaddubsw  %%xmm10,%%xmm3                                  \n" \
  "psubw      %%xmm3,%%xmm2                                   \n" \
  "pmulhuw    %%xmm14,%%xmm4                                  \n" \
  "paddsw     %%xmm4,%%xmm0                                   \n" \
  "paddsw     %%xmm4,%%xmm1                                   \n" \
  "paddsw     %%xmm4,%%xmm2                                   \n"
#define YUVTORGB_REGS \
  "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14",

#else
#define YUVTORGB_SETUP(yuvconstants)
// Convert 8 pixels: 8 UV and 8 Y
#define YUVTORGB16(yuvconstants)                                  \
  "movdqa     %%xmm0,%%xmm1                                   \n" \
  "movdqa     %%xmm0,%%xmm2                                   \n" \
  "movdqa     %%xmm0,%%xmm3                                   \n" \
  "movdqa     96(%[yuvconstants]),%%xmm0                      \n" \
  "pmaddubsw  (%[yuvconstants]),%%xmm1                        \n" \
  "psubw      %%xmm1,%%xmm0                                   \n" \
  "movdqa     128(%[yuvconstants]),%%xmm1                     \n" \
  "pmaddubsw  32(%[yuvconstants]),%%xmm2                      \n" \
  "psubw      %%xmm2,%%xmm1                                   \n" \
  "movdqa     160(%[yuvconstants]),%%xmm2                     \n" \
  "pmaddubsw  64(%[yuvconstants]),%%xmm3                      \n" \
  "psubw      %%xmm3,%%xmm2                                   \n" \
  "pmulhuw    192(%[yuvconstants]),%%xmm4                     \n" \
  "paddsw     %%xmm4,%%xmm0                                   \n" \
  "paddsw     %%xmm4,%%xmm1                                   \n" \
  "paddsw     %%xmm4,%%xmm2                                   \n"
#define YUVTORGB_REGS
#endif

#define YUVTORGB(yuvconstants)                                    \
  YUVTORGB16(yuvconstants)                                        \
  "psraw      $0x6,%%xmm0                                     \n" \
  "psraw      $0x6,%%xmm1                                     \n" \
  "psraw      $0x6,%%xmm2                                     \n" \
  "packuswb   %%xmm0,%%xmm0                                   \n" \
  "packuswb   %%xmm1,%%xmm1                                   \n" \
  "packuswb   %%xmm2,%%xmm2                                   \n"

// Store 8 ARGB values.
#define STOREARGB                                                  \
  "punpcklbw  %%xmm1,%%xmm0                                    \n" \
  "punpcklbw  %%xmm5,%%xmm2                                    \n" \
  "movdqa     %%xmm0,%%xmm1                                    \n" \
  "punpcklwd  %%xmm2,%%xmm0                                    \n" \
  "punpckhwd  %%xmm2,%%xmm1                                    \n" \
  "movdqu     %%xmm0,(%[dst_argb])                             \n" \
  "movdqu     %%xmm1,0x10(%[dst_argb])                         \n" \
  "lea        0x20(%[dst_argb]), %[dst_argb]                   \n"

void OMITFP NV21ToARGBRow_SSSE3(const uint8_t* y_buf,
    const uint8_t* vu_buf,
    uint8_t* dst_argb,
    const struct YuvConstants* yuvconstants,
    int width) {
    // clang-format off
    asm volatile (
        YUVTORGB_SETUP(yuvconstants)
        "pcmpeqb   %%xmm5,%%xmm5                   \n"

        LABELALIGN
        "1:                                        \n"
        READNV21
        YUVTORGB(yuvconstants)
        STOREARGB
        "sub       $0x8,%[width]                   \n"
        "jg        1b                              \n"
        : [y_buf] "+r"(y_buf),    // %[y_buf]
        [vu_buf]"+r"(vu_buf),    // %[vu_buf]
        [dst_argb]"+r"(dst_argb),  // %[dst_argb]
        [width]"+rm"(width)    // %[width]
        : [yuvconstants] "r"(yuvconstants), // %[yuvconstants]
        [kShuffleNV21]"m"(kShuffleNV21)
        : "memory", "cc", YUVTORGB_REGS
        "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
        );
    // clang-format on
}

#endif  // HAS_I422TOARGBROW_SSSE3

// Read 8 VU from NV21, upsample to 16 UV.
#define READNV21_AVX2                                                 \
  "vmovdqu    (%[vu_buf]),%%xmm0                                  \n" \
  "lea        0x10(%[vu_buf]),%[vu_buf]                           \n" \
  "vpermq     $0xd8,%%ymm0,%%ymm0                                 \n" \
  "vpshufb     %[kShuffleNV21], %%ymm0, %%ymm0                    \n" \
  "vmovdqu    (%[y_buf]),%%xmm4                                   \n" \
  "vpermq     $0xd8,%%ymm4,%%ymm4                                 \n" \
  "vpunpcklbw %%ymm4,%%ymm4,%%ymm4                                \n" \
  "lea        0x10(%[y_buf]),%[y_buf]                             \n"

#if defined(__x86_64__)
#define YUVTORGB_SETUP_AVX2(yuvconstants)                            \
  "vmovdqa     (%[yuvconstants]),%%ymm8                          \n" \
  "vmovdqa     32(%[yuvconstants]),%%ymm9                        \n" \
  "vmovdqa     64(%[yuvconstants]),%%ymm10                       \n" \
  "vmovdqa     96(%[yuvconstants]),%%ymm11                       \n" \
  "vmovdqa     128(%[yuvconstants]),%%ymm12                      \n" \
  "vmovdqa     160(%[yuvconstants]),%%ymm13                      \n" \
  "vmovdqa     192(%[yuvconstants]),%%ymm14                      \n"

#define YUVTORGB16_AVX2(yuvconstants)                                 \
  "vpmaddubsw  %%ymm10,%%ymm0,%%ymm2                              \n" \
  "vpmaddubsw  %%ymm9,%%ymm0,%%ymm1                               \n" \
  "vpmaddubsw  %%ymm8,%%ymm0,%%ymm0                               \n" \
  "vpsubw      %%ymm2,%%ymm13,%%ymm2                              \n" \
  "vpsubw      %%ymm1,%%ymm12,%%ymm1                              \n" \
  "vpsubw      %%ymm0,%%ymm11,%%ymm0                              \n" \
  "vpmulhuw    %%ymm14,%%ymm4,%%ymm4                              \n" \
  "vpaddsw     %%ymm4,%%ymm0,%%ymm0                               \n" \
  "vpaddsw     %%ymm4,%%ymm1,%%ymm1                               \n" \
  "vpaddsw     %%ymm4,%%ymm2,%%ymm2                               \n"

#define YUVTORGB_REGS_AVX2 \
  "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14",

#else  // Convert 16 pixels: 16 UV and 16 Y.
#define YUVTORGB_SETUP_AVX2(yuvconstants)
#define YUVTORGB16_AVX2(yuvconstants)                                 \
  "vpmaddubsw  64(%[yuvconstants]),%%ymm0,%%ymm2                  \n" \
  "vpmaddubsw  32(%[yuvconstants]),%%ymm0,%%ymm1                  \n" \
  "vpmaddubsw  (%[yuvconstants]),%%ymm0,%%ymm0                    \n" \
  "vmovdqu     160(%[yuvconstants]),%%ymm3                        \n" \
  "vpsubw      %%ymm2,%%ymm3,%%ymm2                               \n" \
  "vmovdqu     128(%[yuvconstants]),%%ymm3                        \n" \
  "vpsubw      %%ymm1,%%ymm3,%%ymm1                               \n" \
  "vmovdqu     96(%[yuvconstants]),%%ymm3                         \n" \
  "vpsubw      %%ymm0,%%ymm3,%%ymm0                               \n" \
  "vpmulhuw    192(%[yuvconstants]),%%ymm4,%%ymm4                 \n" \
  "vpaddsw     %%ymm4,%%ymm0,%%ymm0                               \n" \
  "vpaddsw     %%ymm4,%%ymm1,%%ymm1                               \n" \
  "vpaddsw     %%ymm4,%%ymm2,%%ymm2                               \n"
#define YUVTORGB_REGS_AVX2
#endif

#define YUVTORGB_AVX2(yuvconstants)                                   \
  YUVTORGB16_AVX2(yuvconstants)                                       \
  "vpsraw      $0x6,%%ymm0,%%ymm0                                 \n" \
  "vpsraw      $0x6,%%ymm1,%%ymm1                                 \n" \
  "vpsraw      $0x6,%%ymm2,%%ymm2                                 \n" \
  "vpackuswb   %%ymm0,%%ymm0,%%ymm0                               \n" \
  "vpackuswb   %%ymm1,%%ymm1,%%ymm1                               \n" \
  "vpackuswb   %%ymm2,%%ymm2,%%ymm2                               \n"

// Store 16 ARGB values.
#define STOREARGB_AVX2                                                \
  "vpunpcklbw %%ymm1,%%ymm0,%%ymm0                                \n" \
  "vpermq     $0xd8,%%ymm0,%%ymm0                                 \n" \
  "vpunpcklbw %%ymm5,%%ymm2,%%ymm2                                \n" \
  "vpermq     $0xd8,%%ymm2,%%ymm2                                 \n" \
  "vpunpcklwd %%ymm2,%%ymm0,%%ymm1                                \n" \
  "vpunpckhwd %%ymm2,%%ymm0,%%ymm0                                \n" \
  "vmovdqu    %%ymm1,(%[dst_argb])                                \n" \
  "vmovdqu    %%ymm0,0x20(%[dst_argb])                            \n" \
  "lea       0x40(%[dst_argb]), %[dst_argb]                       \n"


#if defined(HAS_NV21TOARGBROW_AVX2)
// 16 pixels.
// 8 VU values upsampled to 16 UV, mixed with 16 Y producing 16 ARGB (64 bytes).
void OMITFP NV21ToARGBRow_AVX2(const uint8_t* y_buf,
    const uint8_t* vu_buf,
    uint8_t* dst_argb,
    const struct YuvConstants* yuvconstants,
    int width) {
    // clang-format off
    asm volatile (
        YUVTORGB_SETUP_AVX2(yuvconstants)
        "vpcmpeqb   %%ymm5,%%ymm5,%%ymm5           \n"

        LABELALIGN
        "1:                                        \n"
        READNV21_AVX2
        YUVTORGB_AVX2(yuvconstants)
        STOREARGB_AVX2
        "sub       $0x10,%[width]                  \n"
        "jg        1b                              \n"
        "vzeroupper                                \n"
        : [y_buf] "+r"(y_buf),    // %[y_buf]
        [vu_buf]"+r"(vu_buf),    // %[vu_buf]
        [dst_argb]"+r"(dst_argb),  // %[dst_argb]
        [width]"+rm"(width)    // %[width]
        : [yuvconstants] "r"(yuvconstants), // %[yuvconstants]
        [kShuffleNV21]"m"(kShuffleNV21)
        : "memory", "cc", YUVTORGB_REGS_AVX2
        "xmm0", "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
        );
    // clang-format on
}
#endif  // HAS_NV21TOARGBROW_AVX2

#endif  // defined(__x86_64__) || defined(__i386__)
#ifdef __cplusplus
	}  // extern "C"
}  // namespace libyuv
#endif