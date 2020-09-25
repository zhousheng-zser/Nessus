/*
 *  Copyright 2011 The LibYuv Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS. All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "libyuv/convert_argb.h"

#include "libyuv/cpu_id.h"
//#ifdef HAVE_JPEG
//#include "libyuv/mjpeg_decoder.h"
//#endif
//#include "libyuv/planar_functions.h"  // For CopyPlane and ARGBShuffle.
//#include "libyuv/rotate_argb.h"
#include "libyuv/row.h"
//#include "libyuv/video_common.h"

#ifdef __cplusplus
namespace libyuv {
    extern "C" {
#endif

// Convert NV21 to RGB24 with matrix
static int NV21ToRGB24Matrix(const uint8_t* src_y,
    int src_stride_y,
    const uint8_t* src_vu,
    int src_stride_vu,
    uint8_t* dst_rgb24,
    int dst_stride_rgb24,
    const struct YuvConstants* yuvconstants,
    int width,
    int height) {
    int y;
    void (*NV21ToRGB24Row)(
        const uint8_t * y_buf, const uint8_t * uv_buf, uint8_t * rgb_buf,
        const struct YuvConstants* yuvconstants, int width) = NV21ToRGB24Row_C;
    if (!src_y || !src_vu || !dst_rgb24 || width <= 0 || height == 0) {
        return -1;
    }
    // Negative height means invert the image.
    if (height < 0) {
        height = -height;
        dst_rgb24 = dst_rgb24 + (height - 1) * dst_stride_rgb24;
        dst_stride_rgb24 = -dst_stride_rgb24;
    }
#if defined(HAS_NV21TORGB24ROW_NEON)
    if (TestCpuFlag(kCpuHasNEON)) {
        NV21ToRGB24Row = NV21ToRGB24Row_Any_NEON;
        if (IS_ALIGNED(width, 8)) {
            NV21ToRGB24Row = NV21ToRGB24Row_NEON;
        }
    }
#endif
#if defined(HAS_NV21TORGB24ROW_SSSE3)
    if (TestCpuFlag(kCpuHasSSSE3)) {
        NV21ToRGB24Row = NV21ToRGB24Row_Any_SSSE3;
        if (IS_ALIGNED(width, 16)) {
            NV21ToRGB24Row = NV21ToRGB24Row_SSSE3;
        }
    }
#endif
#if defined(HAS_NV21TORGB24ROW_AVX2)
    if (TestCpuFlag(kCpuHasAVX2)) {
        NV21ToRGB24Row = NV21ToRGB24Row_Any_AVX2;
        if (IS_ALIGNED(width, 32)) {
            NV21ToRGB24Row = NV21ToRGB24Row_AVX2;
        }
    }
#endif

    for (y = 0; y < height; ++y) {
        NV21ToRGB24Row(src_y, src_vu, dst_rgb24, yuvconstants, width);
        dst_rgb24 += dst_stride_rgb24;
        src_y += src_stride_y;
        if (y & 1) {
            src_vu += src_stride_vu;
        }
    }
    return 0;
}


// Convert NV21 to RGB24.
LIBYUV_API
int NV21ToRGB24(const uint8_t* src_y,
    int src_stride_y,
    const uint8_t* src_vu,
    int src_stride_vu,
    uint8_t* dst_rgb24,
    int dst_stride_rgb24,
    int width,
    int height) {
    return NV21ToRGB24Matrix(src_y, src_stride_y, src_vu, src_stride_vu,
        dst_rgb24, dst_stride_rgb24, &kYuvI601Constants,
        width, height);
}

#ifdef __cplusplus
    }  // extern "C"
}  // namespace libyuv
#endif