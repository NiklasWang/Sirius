#include "common.h"
#include "PreviewClient.h"
#include "SiriusClientCore.h"

namespace sirius {

PreviewClient::PreviewClient() :
    mModule(MODULE_PREVIEW_REQUEST),
    mType(PREVIEW_NV21),
    mName("Preview(NV21) Request"),
    mCore(NULL)
{
}

PreviewClient::~PreviewClient()
{
    if (NOTNULL(mCore)) {
        destruct();
    }
}

int32_t PreviewClient::construct()
{
    int32_t rc = NO_ERROR;

    if (ISNULL(mCore)) {
        mCore = SiriusClientCore::getInstance();
        if (ISNULL(mCore)) {
            LOGE(mModule, "Failed to construct preview request");
            rc = UNKNOWN_ERROR;
        }
    }

    return rc;
}

int32_t PreviewClient::destruct()
{
    if (NOTNULL(mCore)) {
        mCore = NULL;
        SiriusClientCore::removeInstance();
    }

    return NO_ERROR;
}

int32_t PreviewClient::onPreviewFrameReady(
    int32_t w, int32_t h, int32_t stride, int32_t scanline, void *data)
{
    int32_t rc = NO_ERROR;

    if (ISNULL(mCore)) {
        LOGE(mModule, "Sirius client core not constructed.");
    }

    if (SUCCEED(rc)) {
        mLastSize.w = w;
        mLastSize.h = h;
        mLastSize.stride = stride;
        mLastSize.scanline = scanline;
        mLastSize.size = w * h * 3 / 2;
        mLastSize.format = PREVIEW_FORMAT_NV21;
        mLastSize.data = data;
    }

    if (SUCCEED(rc)) {
        rc = mCore->onDataReady(this, type());
        if (!SUCCEED(rc)) {
            LOGD(mModule, "Failed to copy frame data, %d", rc);
        }
    }

    return rc;
}

int32_t PreviewClient::copyData(void *dst, int32_t maxSize)
{
    int32_t rc = NO_ERROR;

    if (mLastSize.size > maxSize) {
        LOGE(mModule, "Insufficient memory to copy data, %d / %d",
            mLastSize.size, maxSize);
        rc = NO_MEMORY;
    }

    if (SUCCEED(rc)) {
        rc = removeStrideCopyFrame(dst);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to remove stride and copy frame, %d", rc);
        }
    }

    return rc;
}

int32_t PreviewClient::sizeOfHeader()
{
    return sizeof(PreviewSize);
}

RequestType PreviewClient::type()
{
    return mType;
}

const char *PreviewClient::name()
{
    return mName;
}

#define ALIGN_WITH_NUM(num, stride) \
    (((num) + (stride) - 1) & (~((stride) - 1)))

int32_t PreviewClient::removeStrideCopyFrame(void *_dst)
{
    uint32_t offset_src = 0;
    uint32_t offset_dst = 0;
    int8_t *src = static_cast<int8_t *>(mLastSize.data);
    int8_t *dst = static_cast<int8_t *>(_dst);

    for (int32_t i = 0; i < mLastSize.h; i++) {
        memcpy(dst + offset_dst, src + offset_src, mLastSize.w);
        offset_dst += mLastSize.w;
        offset_src += ALIGN_WITH_NUM(mLastSize.w, mLastSize.stride);
    }

    offset_src = ALIGN_WITH_NUM(mLastSize.w, mLastSize.stride) *
        ALIGN_WITH_NUM(mLastSize.w, mLastSize.scanline);
    for (int32_t i = 0; i < (mLastSize.h / 2); i++) {
        memcpy(dst + offset_dst, src + offset_src, mLastSize.w);
        offset_dst += mLastSize.w;
        offset_src += ALIGN_WITH_NUM(mLastSize.w, mLastSize.stride);
    }

    return NO_ERROR;
}

};

