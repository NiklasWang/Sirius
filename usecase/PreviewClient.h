#ifndef _PREVIEW_REQUEST_CLIENT_H_
#define _PREVIEW_REQUEST_CLIENT_H_

#include "modules.h"
#include "SiriusIntf.h"
#include "ClientRequestIntf.h"

namespace sirius {

class SiriusClientCore;

class PreviewClient :
    public noncopyable {
public:
    int32_t onPreviewFrameReady(int32_t w, int32_t h,
        int32_t stride, int32_t scanline, void *data);

public:
    virtual int32_t copyData(void *dst, int32_t maxSize) override;
    virtual int32_t sizeOfHeader() override;
    virtual RequestType type() override;
    virtual const char *name() override;

public:
    int32_t construct();
    int32_t destruct();
    PreviewClient();
    virtual ~PreviewClient();

private:
    int32_t removeStrideCopyFrame(void *dst);

private:
    ModuleType  mModule;
    RequestType mType;
    const char * const mName;
    PreviewSize        mLastSize;
    SiriusClientCore  *mCore;
};

};

#endif
