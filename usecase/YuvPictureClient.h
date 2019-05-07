#ifndef _YUV_PICTURE_REQUEST_CLIENT_H_
#define _YUV_PICTURE_REQUEST_CLIENT_H_

#include "modules.h"
#include "SiriusServerIntf.h"
#include "common.h"
#include "RequestHandlerClient.h"

namespace sirius {

class YuvPictureClient :
    public RequestHandlerClient,
    public noncopyable {
public:
    int32_t onYuvPictureReady(int32_t w, int32_t h,
        int32_t stride, int32_t scanline, void *data, int64_t ts = 0LL);

public:
    int32_t sizeOfHeader() override;
    int32_t sizeOfData(void *header) override;
    int32_t copyDataToServer(uint8_t *dst, void *header, uint8_t *src) override;

public:
    int32_t construct();
    int32_t destruct();
    YuvPictureClient();
    virtual ~YuvPictureClient();

private:
    int32_t CopyFrameWithoutStride(
        uint8_t *dst, PictureNV21Header *header, uint8_t *src);

private:
    ModuleType  mModule;
};

};

#endif
