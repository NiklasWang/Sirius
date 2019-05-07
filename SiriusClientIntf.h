#ifndef _SIRIUS_CLIENT_INTF_H_
#define _SIRIUS_CLIENT_INTF_H_

#include "TimeEx.h"
#include "SiriusServerIntf.h"

namespace sirius {

class SiriusClientIntf {
public:
    virtual int32_t init(Header &header) = 0;

    virtual int32_t onPreviewReady(int32_t w, int32_t h, int32_t stride,
        int32_t scanline, void *data, int64_t ts = currentUs()) = 0;

    virtual int32_t onYuvPictureReady(int32_t w, int32_t h, int32_t stride,
        int32_t scanline, void *data, int64_t ts = currentUs()) = 0;

    virtual int32_t onBayerPictureReady(int32_t w, int32_t h,
        void *data, int64_t ts = currentUs(), Pattern pattern = BAYER_PATTERN_RGGB) = 0;

    virtual int32_t sendEvent(int32_t evt, int32_t arg1, int32_t arg2) = 0;

    virtual int32_t sendData(int32_t type, void *data, int32_t size) = 0;

    virtual int32_t abort(int32_t type) = 0;

public:
    virtual ~SiriusClientIntf() {}
};

};

#endif
