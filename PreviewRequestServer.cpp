#include "PreviewRequestServer.h"
#include "configuration.h"

namespace sirius {

int32_t PreviewRequestServer::getPrivateDataSize()
{
    return sizeof(PreviewSize);
}

int32_t PreviewRequestServer::getRequestDataSize()
{
    return 1920 * 1080 * 1.5; // TODO: Different preview size
}

int32_t PreviewRequestServer::copyPrivateData(
    void *dst, void *src, int32_t *size)
{
    *size = getPrivateDataSize();
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

