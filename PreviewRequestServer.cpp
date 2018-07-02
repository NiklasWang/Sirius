#include "PreviewRequestServer.h"
#include "configuration.h"

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

int32_t PreviewRequestServer::copyHeader(
    void *dst, void *src, int32_t *size)
{
    *size = getHeaderSize();
    memcpy(dst, src, *size);
    return NO_ERROR;
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

