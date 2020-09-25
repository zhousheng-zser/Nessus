/*
 *  Copyright 2012 The LibYuv Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS. All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "libyuv/row.h"

#include <string.h>  // For memset.

#include "libyuv/basic_types.h"

#ifdef __cplusplus
namespace libyuv {
	extern "C" {
#endif

// memset for temp is meant to clear the source buffer (not dest) so that
// SIMD that reads full multiple of 16 bytes will not trigger msan errors.
// memset is not needed for production, as the garbage values are processed but
// not used, although there may be edge cases for subsampling.
// The size of the buffer is based on the largest read, which can be inferred
// by the source type (e.g. ARGB) and the mask (last parameter), or by examining
// the source code for how much the source pointers are advanced.

// Subsampled source needs to be increase by 1 of not even.
#define SS(width, shift) (((width) + (1 << (shift)) - 1) >> (shift))

// Any 2 planes to 1 with yuvconstants
#define ANY21C(NAMEANY, ANY_SIMD, UVSHIFT, SBPP, SBPP2, BPP, MASK)            \
  void NAMEANY(const uint8_t* y_buf, const uint8_t* uv_buf, uint8_t* dst_ptr, \
               const struct YuvConstants* yuvconstants, int width) {          \
    SIMD_ALIGNED(uint8_t temp[128 * 3]);                                      \
    memset(temp, 0, 128 * 2); /* for msan */                                  \
    int r = width & MASK;                                                     \
    int n = width & ~MASK;                                                    \
    if (n > 0) {                                                              \
      ANY_SIMD(y_buf, uv_buf, dst_ptr, yuvconstants, n);                      \
    }                                                                         \
    memcpy(temp, y_buf + n * SBPP, r * SBPP);                                 \
    memcpy(temp + 128, uv_buf + (n >> UVSHIFT) * SBPP2,                       \
           SS(r, UVSHIFT) * SBPP2);                                           \
    ANY_SIMD(temp, temp + 128, temp + 256, yuvconstants, MASK + 1);           \
    memcpy(dst_ptr + n * BPP, temp + 256, r * BPP);                           \
  }

#ifdef HAS_NV21TORGB24ROW_NEON
ANY21C(NV21ToRGB24Row_Any_NEON, NV21ToRGB24Row_NEON, 1, 1, 2, 3, 7)
#endif
#ifdef HAS_NV21TORGB24ROW_SSSE3
ANY21C(NV21ToRGB24Row_Any_SSSE3, NV21ToRGB24Row_SSSE3, 1, 1, 2, 3, 15)
#endif
#ifdef HAS_NV21TORGB24ROW_AVX2
ANY21C(NV21ToRGB24Row_Any_AVX2, NV21ToRGB24Row_AVX2, 1, 1, 2, 3, 31)
#endif

#ifdef __cplusplus
	}  // extern "C"
}  // namespace libyuv
#endif