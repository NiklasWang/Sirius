#include "YuvPictureRequestServer.h"
#include "configuration.h"

namespace sirius {

int32_t YuvPictureRequestServer::getPrivateDataSize()
{
    return sizeof(PictureSize);
}

int32_t YuvPictureRequestServer::getRequestDataSize()
{
    return 3264 * 2448 * 1.5; // TODO: Different picture size
}

int32_t YuvPictureRequestServer::copyPrivateData(
    void *dst, void *src, int32_t *size)
{
    *size = getPrivateDataSize();
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

