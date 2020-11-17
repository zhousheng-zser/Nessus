/*
 *  Copyright 2011 The LibYuv Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS. All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef INCLUDE_LIBYUV_ROW_H_
#define INCLUDE_LIBYUV_ROW_H_

 //#include <stdlib.h>  // For malloc.

 //#include "libyuv/basic_types.h"
#include "basic_types.h"

#ifdef __cplusplus
namespace libyuv {
    extern "C" {
#endif

#if defined(__pnacl__) || defined(__CLR_VER) ||            \
    (defined(__native_client__) && defined(__x86_64__)) || \
    (defined(__i386__) && !defined(__SSE__) && !defined(__clang__))
#define LIBYUV_DISABLE_X86
#endif
#if defined(__native_client__)
#define LIBYUV_DISABLE_NEON
#endif
        // MemorySanitizer does not support assembly code yet. http://crbug.com/344505
#if defined(__has_feature)
#if __has_feature(memory_sanitizer)
#define LIBYUV_DISABLE_X86
#endif
#endif

// clang >= 3.5.0 required for Arm64.
#if defined(__clang__) && defined(__aarch64__) && !defined(LIBYUV_DISABLE_NEON)
#if (__clang_major__ < 3) || (__clang_major__ == 3 && (__clang_minor__ < 5))
#define LIBYUV_DISABLE_NEON
#endif  // clang >= 3.5
#endif  // __clang__

// GCC >= 4.7.0 required for AVX2.
#if defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
#if (__GNUC__ > 4) || (__GNUC__ == 4 && (__GNUC_MINOR__ >= 7))
#define GCC_HAS_AVX2 1
#endif  // GNUC >= 4.7
#endif  // __GNUC__

// clang >= 3.4.0 required for AVX2.
#if defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#if (__clang_major__ > 3) || (__clang_major__ == 3 && (__clang_minor__ >= 4))
#define CLANG_HAS_AVX2 1
#endif  // clang >= 3.4
#endif  // __clang__

// Visual C 2012 required for AVX2.
#if defined(_M_IX86) && !defined(__clang__) && defined(_MSC_VER) && \
    _MSC_VER >= 1700
#define VISUALC_HAS_AVX2 1
#endif  // VisualStudio >= 2012

// The following are available on all x86 platforms:
#if !defined(LIBYUV_DISABLE_X86) && \
    (defined(_M_IX86) || defined(__x86_64__) || defined(__i386__))
// Conversions:
#define HAS_ARGBTOYROW_SSSE3
#define HAS_I422TOARGBROW_SSSE3
#define HAS_RGB24TOARGBROW_SSSE3
#define HAS_NV21TORGB24ROW_SSSE3
#endif

// The following are available on all x86 platforms, but
// require VS2012, clang 3.4 or gcc 4.7.
#if !defined(LIBYUV_DISABLE_X86) &&                          \
    (defined(VISUALC_HAS_AVX2) || defined(CLANG_HAS_AVX2) || \
     defined(GCC_HAS_AVX2))
#define HAS_I422TOARGBROW_AVX2
#define HAS_NV21TORGB24ROW_AVX2
#define HAS_NV21TOARGBROW_AVX2
#endif

// The following are also available on x64 Visual C.
#if !defined(LIBYUV_DISABLE_X86) && defined(_MSC_VER) && defined(_M_X64) && \
    (!defined(__clang__) || defined(__SSSE3__))
#define HAS_I422TOARGBROW_SSSE3
#endif

// The following are available for AVX2 gcc/clang x86 platforms:
// TODO(fbarchard): Port to Visual C
#if !defined(LIBYUV_DISABLE_X86) &&                                       \
    (defined(__x86_64__) || (defined(__i386__) && !defined(_MSC_VER))) && \
    (defined(CLANG_HAS_AVX2) || defined(GCC_HAS_AVX2))
#define HAS_ARGBTORGB24ROW_AVX2
#endif

// The following are available on Neon platforms:
#if !defined(LIBYUV_DISABLE_NEON) && \
    (defined(__aarch64__) || defined(__ARM_NEON__) || defined(LIBYUV_NEON))
#define HAS_NV21TORGB24ROW_NEON
#endif

#if defined(_MSC_VER) && !defined(__CLR_VER) && !defined(__clang__)
#if defined(VISUALC_HAS_AVX2)
#define SIMD_ALIGNED(var) __declspec(align(32)) var
#else
#define SIMD_ALIGNED(var) __declspec(align(16)) var
#endif
        typedef __declspec(align(16)) int16_t vec16[8];
        typedef __declspec(align(16)) int32_t vec32[4];
        typedef __declspec(align(16)) float vecf32[4];
        typedef __declspec(align(16)) int8_t vec8[16];
        typedef __declspec(align(16)) uint16_t uvec16[8];
        typedef __declspec(align(16)) uint32_t uvec32[4];
        typedef __declspec(align(16)) uint8_t uvec8[16];
        typedef __declspec(align(32)) int16_t lvec16[16];
        typedef __declspec(align(32)) int32_t lvec32[8];
        typedef __declspec(align(32)) int8_t lvec8[32];
        typedef __declspec(align(32)) uint16_t ulvec16[16];
        typedef __declspec(align(32)) uint32_t ulvec32[8];
        typedef __declspec(align(32)) uint8_t ulvec8[32];
#elif !defined(__pnacl__) && (defined(__GNUC__) || defined(__clang__))
// Caveat GCC 4.2 to 4.7 have a known issue using vectors with const.
#if defined(CLANG_HAS_AVX2) || defined(GCC_HAS_AVX2)
#define SIMD_ALIGNED(var) var __attribute__((aligned(32)))
#else
#define SIMD_ALIGNED(var) var __attribute__((aligned(16)))
#endif
        typedef int16_t __attribute__((vector_size(16))) vec16;
        typedef int32_t __attribute__((vector_size(16))) vec32;
        typedef float __attribute__((vector_size(16))) vecf32;
        typedef int8_t __attribute__((vector_size(16))) vec8;
        typedef uint16_t __attribute__((vector_size(16))) uvec16;
        typedef uint32_t __attribute__((vector_size(16))) uvec32;
        typedef uint8_t __attribute__((vector_size(16))) uvec8;
        typedef int16_t __attribute__((vector_size(32))) lvec16;
        typedef int32_t __attribute__((vector_size(32))) lvec32;
        typedef int8_t __attribute__((vector_size(32))) lvec8;
        typedef uint16_t __attribute__((vector_size(32))) ulvec16;
        typedef uint32_t __attribute__((vector_size(32))) ulvec32;
        typedef uint8_t __attribute__((vector_size(32))) ulvec8;
#else
#define SIMD_ALIGNED(var) var
        typedef int16_t vec16[8];
        typedef int32_t vec32[4];
        typedef float vecf32[4];
        typedef int8_t vec8[16];
        typedef uint16_t uvec16[8];
        typedef uint32_t uvec32[4];
        typedef uint8_t uvec8[16];
        typedef int16_t lvec16[16];
        typedef int32_t lvec32[8];
        typedef int8_t lvec8[32];
        typedef uint16_t ulvec16[16];
        typedef uint32_t ulvec32[8];
        typedef uint8_t ulvec8[32];
#endif

#if defined(__aarch64__)
        // This struct is for Arm64 color conversion.
        struct YuvConstants {
            uvec16 kUVToRB;
            uvec16 kUVToRB2;
            uvec16 kUVToG;
            uvec16 kUVToG2;
            vec16 kUVBiasBGR;
            vec32 kYToRgb;
        };
#elif defined(__arm__)
        // This struct is for ArmV7 color conversion.
        struct YuvConstants {
            uvec8 kUVToRB;
            uvec8 kUVToG;
            vec16 kUVBiasBGR;
            vec32 kYToRgb;
        };
#else
        // This struct is for Intel color conversion.
        struct YuvConstants {
            int8_t kUVToB[32];
            int8_t kUVToG[32];
            int8_t kUVToR[32];
            int16_t kUVBiasB[16];
            int16_t kUVBiasG[16];
            int16_t kUVBiasR[16];
            int16_t kYToRgb[16];
        };

        // Offsets into YuvConstants structure
#define KUVTOB 0
#define KUVTOG 32
#define KUVTOR 64
#define KUVBIASB 96
#define KUVBIASG 128
#define KUVBIASR 160
#define KYTORGB 192
#endif

// Conversion matrix for YUV to RGB
        extern const struct YuvConstants SIMD_ALIGNED(kYuvI601Constants);  // BT.601

        // Conversion matrix for YVU to BGR
        extern const struct YuvConstants SIMD_ALIGNED(kYvuI601Constants);  // BT.601

#define IS_ALIGNED(p, a) (!((uintptr_t)(p) & ((a)-1)))

#if defined(__APPLE__) || defined(__x86_64__) || defined(__llvm__)
#define OMITFP
#else
#define OMITFP __attribute__((optimize("omit-frame-pointer")))
#endif

// NaCL macros for GCC x86 and x64.
#if defined(__native_client__)
#define LABELALIGN ".p2align 5\n"
#else
#define LABELALIGN
#endif

        void NV21ToRGB24Row_NEON(const uint8_t* src_y,
            const uint8_t* src_vu,
            uint8_t* dst_rgb24,
            const struct YuvConstants* yuvconstants,
            int width);

        //implement in row_gcc.cc
        void ARGBToRGB24Row_SSSE3(const uint8_t* src, uint8_t* dst, int width);
        void ARGBToRGB24Row_AVX2(const uint8_t* src, uint8_t* dst, int width);

        //implement in row_common.cc
        void NV21ToRGB24Row_C(const uint8_t* src_y,
            const uint8_t* src_vu,
            uint8_t* rgb_buf,
            const struct YuvConstants* yuvconstants,
            int width);
        void NV21ToRGB24Row_SSSE3(const uint8_t* src_y,
            const uint8_t* src_vu,
            uint8_t* dst_rgb24,
            const struct YuvConstants* yuvconstants,
            int width);
        void NV21ToRGB24Row_AVX2(const uint8_t* src_y,
            const uint8_t* src_vu,
            uint8_t* dst_rgb24,
            const struct YuvConstants* yuvconstants,
            int width);

        //implement in row_gcc.cc
        void NV21ToARGBRow_SSSE3(const uint8_t* y_buf,
            const uint8_t* vu_buf,
            uint8_t* dst_argb,
            const struct YuvConstants* yuvconstants,
            int width);
        void NV21ToARGBRow_AVX2(const uint8_t* y_buf,
            const uint8_t* vu_buf,
            uint8_t* dst_argb,
            const struct YuvConstants* yuvconstants,
            int width);

        //implement in row_any.cc
        void NV21ToRGB24Row_Any_SSSE3(const uint8_t* y_buf,
            const uint8_t* uv_buf,
            uint8_t* dst_ptr,
            const struct YuvConstants* yuvconstants,
            int width);
        void NV21ToRGB24Row_Any_AVX2(const uint8_t* y_buf,
            const uint8_t* uv_buf,
            uint8_t* dst_ptr,
            const struct YuvConstants* yuvconstants,
            int width);
        void NV21ToRGB24Row_Any_NEON(const uint8_t* y_buf,
            const uint8_t* uv_buf,
            uint8_t* dst_ptr,
            const struct YuvConstants* yuvconstants,
            int width);
#ifdef __cplusplus
    }  // extern "C"
}  // namespace libyuv
#endif

#endif  // INCLUDE_LIBYUV_ROW_H_