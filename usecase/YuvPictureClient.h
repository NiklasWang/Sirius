#ifndef _YUV_PICTURE_REQUEST_CLIENT_H_
#define _YUV_PICTURE_REQUEST_CLIENT_H_

#include "modules.h"
#include "SiriusIntf.h"
#include "ClientRequestIntf.h"

namespace sirius {

class SiriusClientCore;

class YuvPictureRequestClient :
    public ClientRequestIntf {
public:
    int32_t onYuvPictureFrameReady(int32_t w, int32_t h,
        int32_t stride, int32_t scanline, void *data);

public:
    virtual int32_t copyHeader(void *dst, int32_t maxSize) override;
    virtual int32_t copyData(void *dst, int32_t maxSize) override;
    virtual int32_t sizeOfHeader() override;
    virtual RequestType type() override;
    virtual const char *name() override;

public:
    int32_t construct();
    int32_t destruct();
    YuvPictureRequestClient();
    virtual ~YuvPictureRequestClient();

private:
    int32_t removeStrideCopyFrame(void *dst);

private:
    YuvPictureRequestClient(const YuvPictureRequestClient &rhs);
    YuvPictureRequestClient &operator=(const YuvPictureRequestClient &rhs);

private:
    enum PictureFormat {
        PICTURE_FORMAT_NV21,
    };

    struct YuvPictureSize {
        int32_t w;
        int32_t h;
        int32_t stride;
        int32_t scanline;
        int32_t  size;
        PictureFormat format;
        void   *data;
    };

private:
    ModuleType  mModule;
    RequestType mType;
    const char * const mName;
    YuvPictureSize     mLastSize;
    SiriusClientCore  *mCore;
};

};

#endif
