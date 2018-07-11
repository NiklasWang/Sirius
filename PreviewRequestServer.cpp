#include "PreviewRequestServer.h"

#define PREIVEW_REQUEST_MEMORY_NUM 6

namespace sirius {

int32_t PreviewRequestServer::getHeaderSize()
{
    return sizeof(PreviewNV21Header);
}

int32_t PreviewRequestServer::getDataSize()
{
    // Client will remove stride
    return mHeader.previewW * mHeader.previewH * 3 / 2;
}

PreviewRequestServer::PreviewRequestServer(HandlerOpsIntf *ops) :
    RequestHandler(ops,
        PREVIEW_NV21,
        "Preview(NV21) Request",
        PREIVEW_REQUEST_MEMORY_NUM)
{}

PreviewRequestServer::~PreviewRequestServer()
{
}

};

