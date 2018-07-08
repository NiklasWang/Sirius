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

