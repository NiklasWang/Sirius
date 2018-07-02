#ifndef _PREVIEW_REQUEST_SERVER_H_
#define _PREVIEW_REQUEST_SERVER_H_

#include "common.h"
#include "RequestHandler.h"

namespace sirius {

class PreviewRequestServer :
    public RequestHandler,
    public noncopyable {
public:
    int32_t getPrivateDataSize() override;
    int32_t getRequestDataSize() override;
    int32_t copyPrivateData(void *dst, void *src, int32_t *size) override;

public:
    PreviewRequestServer(HandlerOpsIntf *ops);
    virtual ~PreviewRequestServer();

private:
    enum PreivewFormat {
        PREVIEW_FORMAT_NV21,
    };

    struct PreviewSize {
        int32_t w;
        int32_t h;
        int32_t stride;
        int32_t scanline;
        int32_t size;
        PreivewFormat format;
        void   *data;
    };
};

};

#endif
