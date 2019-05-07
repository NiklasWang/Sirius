#include "common.h"
#include "TimeEx.h"
#include "BayerPictureClient.h"

namespace sirius {

BayerPictureClient::BayerPictureClient() :
    RequestHandlerClient(
        PICTURE_BAYER,
        "Bayer Picture Request Client",
        REQUEST_HANDLER_MAX_MEMORY_NUM),
    mModule(MODULE_BAYER_PICTURE_REQUEST)
{
}

BayerPictureClient::~BayerPictureClient()
{
    destruct();
}

int32_t BayerPictureClient::construct()
{
    return RequestHandlerClient::construct();
}

int32_t BayerPictureClient::destruct()
{
    return RequestHandlerClient::destruct();
}

int32_t BayerPictureClient::sizeOfHeader()
{
    return sizeof(PictureBayerHeader);
}

int32_t BayerPictureClient::sizeOfData(void *header)
{
    PictureBayerHeader *_header =
        static_cast<PictureBayerHeader *>(header);
    return _header->w * _header->h;
}

int32_t BayerPictureClient::onBayerPictureReady(
    int32_t _w, int32_t _h, void *_data, int64_t _ts, Pattern _pattern)
{
    int32_t rc = NO_ERROR;

    PictureBayerHeader header = {
        .w = _w,
        .h = _h,
        .size = _w * _h,
        .ts = _ts == 0LL ? currentUs() : _ts,
        .pattern = _pattern,
    };

    if (SUCCEED(rc)) {
        rc = onDataReady(&header, (uint8_t *)_data);
        if (!SUCCEED(rc)) {
            LOGD(mModule, "Failed to copy frame data, %d", rc);
        }
    }

    return rc;
}

int32_t BayerPictureClient::copyDataToServer(uint8_t *dst, void *header, uint8_t *src)
{
    int32_t rc = NO_ERROR;
    PictureBayerHeader *_header =
        static_cast<PictureBayerHeader *>(header);

    void *_dst = memcpy(dst, src, _header->size);
    if (_dst != dst) {
        LOGE(mModule, "Failed to copy bayer picture.");
        rc = NO_MEMORY;
    }

    return rc;
}

};

