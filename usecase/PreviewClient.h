#ifndef _PREVIEW_REQUEST_CLIENT_H_
#define _PREVIEW_REQUEST_CLIENT_H_

#include "modules.h"
#include "SiriusServerIntf.h"
#include "common.h"
#include "RequestHandlerClient.h"

namespace sirius {

class PreviewClient :
    public RequestHandlerClient,
    public noncopyable {
public:
    int32_t onPreviewReady(int32_t w, int32_t h,
        int32_t stride, int32_t scanline, void *data, int64_t ts = 0LL);

public:
    int32_t sizeOfHeader() override;
    int32_t sizeOfData(void *header) override;
    int32_t copyDataToServer(uint8_t *dst, void *header, uint8_t *src) override;

public:
    int32_t construct();
    int32_t destruct();
    PreviewClient();
    virtual ~PreviewClient();

private:
    int32_t CopyFrameWithoutStride(
        uint8_t *dst, PreviewNV21Header *header, uint8_t *src);

private:
    ModuleType  mModule;
};

};

#endif
