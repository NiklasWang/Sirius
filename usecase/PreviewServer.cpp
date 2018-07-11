#include "PreviewServer.h"

#define PREIVEW_REQUEST_MEMORY_NUM 6

namespace sirius {

int32_t PreviewServer::getHeaderSize()
{
    return sizeof(PreviewNV21Header);
}

int32_t PreviewServer::getDataSize()
{
    // Client will remove stride
    return mHeader.previewW * mHeader.previewH * 3 / 2;
}

PreviewServer::PreviewServer(HandlerOpsIntf *ops) :
    RequestHandler(ops,
        PREVIEW_NV21,
        "Preview(NV21) Request",
        PREIVEW_REQUEST_MEMORY_NUM)
{}

PreviewServer::~PreviewServer()
{
}

};

