#include "common.h"
#include "TimeEx.h"
#include "PreviewClient.h"

namespace sirius {

PreviewClient::PreviewClient() :
    RequestHandlerClient(
        PREVIEW_NV21,
        "Preview(NV21) Request Client",
        REQUEST_HANDLER_MAX_MEMORY_NUM),
    mModule(MODULE_PREVIEW_REQUEST)
{
}

PreviewClient::~PreviewClient()
{
    destruct();
}

int32_t PreviewClient::construct()
{
    return RequestHandlerClient::construct();
}

int32_t PreviewClient::destruct()
{
    return RequestHandlerClient::destruct();
}

int32_t PreviewClient::sizeOfHeader()
{
    return sizeof(PreviewNV21Header);
}

int32_t PreviewClient::sizeOfData(void *header)
{
    PreviewNV21Header *_header =
        static_cast<PreviewNV21Header *>(header);
    return _header->w * _header->h * 3 / 2;
}

int32_t PreviewClient::onPreviewReady(
    int32_t _w, int32_t _h, int32_t _stride,
    int32_t _scanline, void *_data, int64_t _ts)
{
    int32_t rc = NO_ERROR;

    PreviewNV21Header header = {
        .w = _w,
        .h = _h,
        .stride = _stride,
        .scanline = _scanline,
        .size = _w * _h * 3 / 2,
        .ts = _ts == 0LL ? currentUs() : _ts,
    };

    if (SUCCEED(rc)) {
        rc = onDataReady(&header, (uint8_t *)_data);
        if (!SUCCEED(rc)) {
            LOGD(mModule, "Failed to copy frame data, %d", rc);
        }
    }

    return rc;
}

int32_t PreviewClient::copyDataToServer(uint8_t *dst, void *header, uint8_t *src)
{
    PreviewNV21Header *_header =
        static_cast<PreviewNV21Header *>(header);
    int32_t rc = CopyFrameWithoutStride(dst, _header, src);
    if (!SUCCEED(rc)) {
        LOGE(mModule, "Failed to remove stride and copy frame, %d", rc);
    }

    return rc;
}

#define ALIGN_WITH_NUM(num, stride) \
    (((num) + (stride) - 1) & (~((stride) - 1)))

int32_t PreviewClient::CopyFrameWithoutStride(
    uint8_t *dst, PreviewNV21Header *header, uint8_t *src)
{
    uint32_t offset_src = 0;
    uint32_t offset_dst = 0;

    for (int32_t i = 0; i < header->h; i++) {
        memcpy(dst + offset_dst, src + offset_src, header->w);
        offset_dst += header->w;
        offset_src += ALIGN_WITH_NUM(header->w, header->stride);
    }

    offset_src = ALIGN_WITH_NUM(header->w, header->stride) *
        ALIGN_WITH_NUM(header->w, header->scanline);
    for (int32_t i = 0; i < (header->h / 2); i++) {
        memcpy(dst + offset_dst, src + offset_src, header->w);
        offset_dst += header->w;
        offset_src += ALIGN_WITH_NUM(header->w, header->stride);
    }

    return NO_ERROR;
}

};

