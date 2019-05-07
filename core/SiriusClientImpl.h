#ifndef _SIRIUS_CLIENT_IMPL_H_
#define _SIRIUS_CLIENT_IMPL_H_

#include "SiriusClientIntf.h"
#include "RWLock.h"

namespace sirius {

class PreviewClient;
class YuvPictureClient;
class BayerPictureClient;
class EventClient;
class DataClient;
class SiriusClientCore;

class SiriusClientImpl :
    public SiriusClientIntf
{
public:
    int32_t init(Header &header) override;

    int32_t onPreviewReady(int32_t w, int32_t h, int32_t stride,
        int32_t scanline, void *data, int64_t ts) override;

    int32_t onYuvPictureReady(int32_t w, int32_t h, int32_t stride,
        int32_t scanline, void *data, int64_t ts) override;

    int32_t onBayerPictureReady(int32_t w, int32_t h,
        void *data, int64_t ts, Pattern pattern) override;

    int32_t sendEvent(int32_t evt, int32_t arg1, int32_t arg2) override;

    int32_t sendData(int32_t type, void *data, int32_t size) override;

    int32_t abort(int32_t type) override;

public:
    SiriusClientImpl();
    virtual ~SiriusClientImpl();

private:
    int32_t enablePreview();
    int32_t enableSnapshot();
    int32_t enableBayer();
    int32_t enableEvent();
    int32_t enableData();

private:
    ModuleType          mModule;
    RWLock              mLock;
    PreviewClient      *mPreview;
    YuvPictureClient   *mSnapshot;
    BayerPictureClient *mBayer;
    EventClient        *mEvent;
    DataClient         *mData;
    SiriusClientCore   *mCore;
};

};

#endif
