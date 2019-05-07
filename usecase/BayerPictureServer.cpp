#include "BayerPictureServer.h"

#define BAYER_PICTURE_REQUEST_MEMORY_NUM 1

namespace sirius {

int32_t BayerPictureServer::getHeaderSize()
{
    return sizeof(PictureBayerHeader);
}

int32_t BayerPictureServer::getDataSize()
{
    return mHeader.bayerPictureW * mHeader.bayerPictureH;
}

BayerPictureServer::BayerPictureServer(HandlerOpsIntf *ops) :
    RequestHandler(ops,
        PICTURE_BAYER,
        "Bayer Picture Request",
        BAYER_PICTURE_REQUEST_MEMORY_NUM)
{}

BayerPictureServer::~BayerPictureServer()
{
}

};

