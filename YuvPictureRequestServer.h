#ifndef _YUV_PICTURE_REQUEST_SERVER_H_
#define _YUV_PICTURE_REQUEST_SERVER_H_

#include "common.h"
#include "RequestHandler.h"

namespace sirius {

class YuvPictureRequestServer :
    public RequestHandler,
    public noncopyable {
public:
    int32_t getPrivateDataSize() override;
    int32_t getRequestDataSize() override;
    int32_t copyPrivateData(void *dst, void *src, int32_t *size) override;

public:
    YuvPictureRequestServer(HandlerOpsIntf *ops);
    virtual ~YuvPictureRequestServer();

private:
    enum PictureFormat {
        PICTURE_FORMAT_NV21,
    };

    struct PictureSize {
        int32_t w;
        int32_t h;
        int32_t stride;
        int32_t scanline;
        int32_t  size;
        PictureFormat format;
    };
};

};

#endif
