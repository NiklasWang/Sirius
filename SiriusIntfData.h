#ifndef _SIRIUS_INTF_DATA_H_
#define _SIRIUS_INTF_DATA_H_

#include <stdint.h>

enum RequestType {
    PREVIEW_NV21,
    PICTURE_NV21,
    PICTURE_BAYER,
    EXTENDED_EVENT,
    CUSTOM_DATA,
    REQUEST_TYPE_MAX_INVALID,
};

struct PreviewNV21Header {
    int32_t w;
    int32_t h;
    int32_t stride;
    int32_t scanline;
    int32_t size;
    int64_t ts;
};

struct PictureNV21Header {
    int32_t w;
    int32_t h;
    int32_t stride;
    int32_t scanline;
    int32_t size;
    int64_t ts;
};

enum Pattern {
    BAYER_PATTERN_RGGB,
    BAYER_PATTERN_GRGB,
    BAYER_PATTERN_MAX_INVALID,
};

struct PictureBayerHeader {
    int32_t w;
    int32_t h;
    int32_t size;
    int64_t ts;
    Pattern pattern;
};

struct Header {
    int32_t previewW;
    int32_t previewH;
    int32_t previewStride;
    int32_t previewScanline;
    int32_t pictureW;
    int32_t pictureH;
    int32_t pictureStride;
    int32_t pictureScanline;
    int32_t bayerPictureW;
    int32_t bayerPictureH;
    Pattern bayerPattern;
};

#endif

