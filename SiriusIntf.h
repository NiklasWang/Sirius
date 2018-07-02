#ifndef _SIRIUS_INTF_H_
#define _SIRIUS_INTF_H_

#include <stdint.h>

enum RequestType {
    PREVIEW_NV21,
    PICTURE_NV21,
    PICTURE_BAYER,
    EXTENDED_EVENT,
    REQUEST_TYPE_MAX_INVALID,
};

#define ALL_BUFFERS NULL

typedef int32_t (*callback_func)(RequestType type, void *buf);

class SiriusIntf {
public:
    virtual int32_t request(RequestType type) = 0;
    virtual int32_t abort(RequestType type) = 0;
    virtual int32_t enqueueBuf(RequestType type, void *buf, int32_t size) = 0;
    virtual int32_t dequeueBuf(RequestType type, void *buf = ALL_BUFFERS) = 0;
    virtual int32_t setCallback(callback_func func) = 0;

public:
    virtual ~SiriusIntf() {}
};

#endif
