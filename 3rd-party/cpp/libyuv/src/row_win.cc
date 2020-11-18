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

// This module is for Visual C 32/64 bit and clangcl 32 bit
#if !defined(LIBYUV_DISABLE_X86) && defined(_MSC_VER) && \
    (defined(_M_IX86) || (defined(_M_X64) && !defined(__clang__)))

#if defined(_M_X64)
#include <emmintrin.h>
#include <tmmintrin.h>  // For _mm_maddubs_epi16
#endif

#ifdef __cplusplus
namespace libyuv {
extern "C" {
#endif

// 64 bit
#if defined(_M_X64)
// 32 bit
#else  // defined(_M_X64)
#ifdef HAS_ARGBTOYROW_SSSE3

// Shuffle table for converting ARGB to RGB24.
static const uvec8 kShuffleMaskARGBToRGB24 = {
    0u, 1u, 2u, 4u, 5u, 6u, 8u, 9u, 10u, 12u, 13u, 14u, 128u, 128u, 128u, 128u};


// NV21 shuf 8 VU to 16 UV.
static const lvec8 kShuffleNV21 = {
    1, 0, 1, 0, 3, 2, 3, 2, 5, 4, 5, 4, 7, 6, 7, 6,
    1, 0, 1, 0, 3, 2, 3, 2, 5, 4, 5, 4, 7, 6, 7, 6,
};

__declspec(naked) void ARGBToRGB24Row_SSSE3(const uint8_t* src_argb,
                                            uint8_t* dst_rgb,
                                            int width) {
  __asm {
    mov       eax, [esp + 4]  // src_argb
    mov       edx, [esp + 8]  // dst_rgb
    mov       ecx, [esp + 12]  // width
    movdqa    xmm6, xmmword ptr kShuffleMaskARGBToRGB24

 convertloop:
    movdqu    xmm0, [eax]  // fetch 16 pixels of argb
    movdqu    xmm1, [eax + 16]
    movdqu    xmm2, [eax + 32]
    movdqu    xmm3, [eax + 48]
    lea       eax, [eax + 64]
    pshufb    xmm0, xmm6  // pack 16 bytes of ARGB to 12 bytes of RGB
    pshufb    xmm1, xmm6
    pshufb    xmm2, xmm6
    pshufb    xmm3, xmm6
    movdqa    xmm4, xmm1  // 4 bytes from 1 for 0
    psrldq    xmm1, 4  // 8 bytes from 1
    pslldq    xmm4, 12  // 4 bytes from 1 for 0
    movdqa    xmm5, xmm2  // 8 bytes from 2 for 1
    por       xmm0, xmm4  // 4 bytes from 1 for 0
    pslldq    xmm5, 8  // 8 bytes from 2 for 1
    movdqu    [edx], xmm0  // store 0
    por       xmm1, xmm5  // 8 bytes from 2 for 1
    psrldq    xmm2, 8  // 4 bytes from 2
    pslldq    xmm3, 4  // 12 bytes from 3 for 2
    por       xmm2, xmm3  // 12 bytes from 3 for 2
    movdqu    [edx + 16], xmm1  // store 1
    movdqu    [edx + 32], xmm2  // store 2
    lea       edx, [edx + 48]
    sub       ecx, 16
    jg        convertloop
    ret
  }
}


// Read 8 UV from NV21, upsample to 16 UV.
#define READNV21_AVX2 \
  __asm {                                                  \
    __asm vmovdqu    xmm0, [esi] /* UV */                     \
    __asm lea        esi,  [esi + 16]                                          \
    __asm vpermq     ymm0, ymm0, 0xd8                                          \
    __asm vpshufb    ymm0, ymm0, ymmword ptr kShuffleNV21                      \
    __asm vmovdqu    xmm4, [eax] /* Y */                      \
    __asm vpermq     ymm4, ymm4, 0xd8                                          \
    __asm vpunpcklbw ymm4, ymm4, ymm4                                          \
    __asm lea        eax, [eax + 16]}

// Convert 16 pixels: 16 UV and 16 Y.
#define YUVTORGB_AVX2(YuvConstants) \
  __asm {                                    \
    __asm vpmaddubsw ymm2, ymm0, ymmword ptr [YuvConstants + KUVTOR] /* R UV */\
    __asm vpmaddubsw ymm1, ymm0, ymmword ptr [YuvConstants + KUVTOG] /* G UV */\
    __asm vpmaddubsw ymm0, ymm0, ymmword ptr [YuvConstants + KUVTOB] /* B UV */\
    __asm vmovdqu    ymm3, ymmword ptr [YuvConstants + KUVBIASR]               \
    __asm vpsubw     ymm2, ymm3, ymm2                                          \
    __asm vmovdqu    ymm3, ymmword ptr [YuvConstants + KUVBIASG]               \
    __asm vpsubw     ymm1, ymm3, ymm1                                          \
    __asm vmovdqu    ymm3, ymmword ptr [YuvConstants + KUVBIASB]               \
    __asm vpsubw     ymm0, ymm3, ymm0 /* Step 2: Find Y contribution to 16 R,G,B values */                       \
    __asm vpmulhuw   ymm4, ymm4, ymmword ptr [YuvConstants + KYTORGB]          \
    __asm vpaddsw    ymm0, ymm0, ymm4 /* B += Y */                   \
    __asm vpaddsw    ymm1, ymm1, ymm4 /* G += Y */                   \
    __asm vpaddsw    ymm2, ymm2, ymm4 /* R += Y */                   \
    __asm vpsraw     ymm0, ymm0, 6                                             \
    __asm vpsraw     ymm1, ymm1, 6                                             \
    __asm vpsraw     ymm2, ymm2, 6                                             \
    __asm vpackuswb  ymm0, ymm0, ymm0 /* B */                        \
    __asm vpackuswb  ymm1, ymm1, ymm1 /* G */                        \
    __asm vpackuswb  ymm2, ymm2, ymm2 /* R */                  \
  }

// Store 16 ARGB values.
#define STOREARGB_AVX2 \
  __asm {                                                 \
    __asm vpunpcklbw ymm0, ymm0, ymm1 /* BG */                       \
    __asm vpermq     ymm0, ymm0, 0xd8                                          \
    __asm vpunpcklbw ymm2, ymm2, ymm5 /* RA */                       \
    __asm vpermq     ymm2, ymm2, 0xd8                                          \
    __asm vpunpcklwd ymm1, ymm0, ymm2 /* BGRA first 8 pixels */      \
    __asm vpunpckhwd ymm0, ymm0, ymm2 /* BGRA next 8 pixels */       \
    __asm vmovdqu    0[edx], ymm1                                              \
    __asm vmovdqu    32[edx], ymm0                                             \
    __asm lea        edx,  [edx + 64]}


#ifdef HAS_NV21TOARGBROW_AVX2
// 16 pixels.
// 8 VU values upsampled to 16 UV, mixed with 16 Y producing 16 ARGB (64 bytes).
__declspec(naked) void NV21ToARGBRow_AVX2(
    const uint8_t* y_buf,
    const uint8_t* vu_buf,
    uint8_t* dst_argb,
    const struct YuvConstants* yuvconstants,
    int width) {
  __asm {
    push       esi
    push       ebx
    mov        eax, [esp + 8 + 4]  // Y
    mov        esi, [esp + 8 + 8]  // VU
    mov        edx, [esp + 8 + 12]  // argb
    mov        ebx, [esp + 8 + 16]  // yuvconstants
    mov        ecx, [esp + 8 + 20]  // width
    vpcmpeqb   ymm5, ymm5, ymm5  // generate 0xffffffffffffffff for alpha

 convertloop:
    READNV21_AVX2
    YUVTORGB_AVX2(ebx)
    STOREARGB_AVX2

    sub        ecx, 16
    jg         convertloop

    pop        ebx
    pop        esi
    vzeroupper
    ret
  }
}
#endif  // HAS_NV21TOARGBROW_AVX2

#if defined(HAS_I422TOARGBROW_SSSE3)
// TODO(fbarchard): Read that does half size on Y and treats 420 as 444.
// Allows a conversion with half size scaling.

// Read 4 VU from NV21, upsample to 8 UV.
#define READNV21 \
  __asm {                                                       \
    __asm movq       xmm0, qword ptr [esi] /* UV */                            \
    __asm lea        esi,  [esi + 8]                                           \
    __asm pshufb     xmm0, xmmword ptr kShuffleNV21                            \
    __asm movq       xmm4, qword ptr [eax]                                     \
    __asm punpcklbw  xmm4, xmm4                                                \
    __asm lea        eax, [eax + 8]}

// Convert 8 pixels: 8 UV and 8 Y.
#define YUVTORGB(YuvConstants) \
  __asm {                                         \
    __asm movdqa     xmm1, xmm0                                                \
    __asm movdqa     xmm2, xmm0                                                \
    __asm movdqa     xmm3, xmm0                                                \
    __asm movdqa     xmm0, xmmword ptr [YuvConstants + KUVBIASB]               \
    __asm pmaddubsw  xmm1, xmmword ptr [YuvConstants + KUVTOB]                 \
    __asm psubw      xmm0, xmm1                                                \
    __asm movdqa     xmm1, xmmword ptr [YuvConstants + KUVBIASG]               \
    __asm pmaddubsw  xmm2, xmmword ptr [YuvConstants + KUVTOG]                 \
    __asm psubw      xmm1, xmm2                                                \
    __asm movdqa     xmm2, xmmword ptr [YuvConstants + KUVBIASR]               \
    __asm pmaddubsw  xmm3, xmmword ptr [YuvConstants + KUVTOR]                 \
    __asm psubw      xmm2, xmm3                                                \
    __asm pmulhuw    xmm4, xmmword ptr [YuvConstants + KYTORGB]                \
    __asm paddsw     xmm0, xmm4 /* B += Y */                         \
    __asm paddsw     xmm1, xmm4 /* G += Y */                         \
    __asm paddsw     xmm2, xmm4 /* R += Y */                         \
    __asm psraw      xmm0, 6                                                   \
    __asm psraw      xmm1, 6                                                   \
    __asm psraw      xmm2, 6                                                   \
    __asm packuswb   xmm0, xmm0 /* B */                              \
    __asm packuswb   xmm1, xmm1 /* G */                              \
    __asm packuswb   xmm2, xmm2 /* R */             \
  }

// Store 8 ARGB values.
#define STOREARGB \
  __asm {                                                      \
    __asm punpcklbw  xmm0, xmm1 /* BG */                             \
    __asm punpcklbw  xmm2, xmm5 /* RA */                             \
    __asm movdqa     xmm1, xmm0                                                \
    __asm punpcklwd  xmm0, xmm2 /* BGRA first 4 pixels */            \
    __asm punpckhwd  xmm1, xmm2 /* BGRA next 4 pixels */             \
    __asm movdqu     0[edx], xmm0                                              \
    __asm movdqu     16[edx], xmm1                                             \
    __asm lea        edx,  [edx + 32]}


// 8 pixels.
// 4 UV values upsampled to 8 UV, mixed with 8 Y producing 8 ARGB (32 bytes).
__declspec(naked) void NV21ToARGBRow_SSSE3(
    const uint8_t* y_buf,
    const uint8_t* vu_buf,
    uint8_t* dst_argb,
    const struct YuvConstants* yuvconstants,
    int width) {
  __asm {
    push       esi
    push       ebx
    mov        eax, [esp + 8 + 4]  // Y
    mov        esi, [esp + 8 + 8]  // VU
    mov        edx, [esp + 8 + 12]  // argb
    mov        ebx, [esp + 8 + 16]  // yuvconstants
    mov        ecx, [esp + 8 + 20]  // width
    pcmpeqb    xmm5, xmm5  // generate 0xffffffff for alpha

 convertloop:
    READNV21
    YUVTORGB(ebx)
    STOREARGB

    sub        ecx, 8
    jg         convertloop

    pop        ebx
    pop        esi
    ret
  }
}
#endif  // HAS_I422TOARGBROW_SSSE3

#endif  //HAS_ARGBTOYROW_SSSE3
#endif  // defined(_M_X64)

#ifdef __cplusplus
}  // extern "C"
}  // namespace libyuv
#endif

#endif  // !defined(LIBYUV_DISABLE_X86) && (defined(_M_IX86) || defined(_M_X64))
