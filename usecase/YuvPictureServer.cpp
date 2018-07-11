#include "YuvPictureServer.h"

#define PICTURE_REQUEST_MEMORY_NUM 2

namespace sirius {

int32_t YuvPictureServer::getHeaderSize()
{
    return sizeof(PictureNV21Header);
}

int32_t YuvPictureServer::getDataSize()
{
    return mHeader.pictureW * mHeader.pictureH * 3 / 2;
}

YuvPictureServer::YuvPictureServer(HandlerOpsIntf *ops) :
    RequestHandler(ops,
        PICTURE_NV21,
        "Picture(NV21) Request",
        PICTURE_REQUEST_MEMORY_NUM)
{
}

YuvPictureServer::~YuvPictureServer()
{
}

};

