#include "YuvPictureRequestServer.h"
#include "configuration.h"

namespace sirius {

int32_t YuvPictureRequestServer::getHeaderSize()
{
    return sizeof(PictureNV21Header);
}

int32_t YuvPictureRequestServer::getDataSize()
{
    return mHeader.pictureW * mHeader.pictureH * 3 / 2;
}

int32_t YuvPictureRequestServer::copyHeader(
    void *dst, void *src, int32_t *size)
{
    *size = getHeaderSize();
    memcpy(dst, src, *size);
    return NO_ERROR;
}

YuvPictureRequestServer::YuvPictureRequestServer(HandlerOpsIntf *ops) :
    RequestHandler(ops,
        PICTURE_NV21,
        "Picture(NV21) Request",
        PICTURE_REQUEST_MEMORY_NUM)
{
}

YuvPictureRequestServer::~YuvPictureRequestServer()
{
}

};

