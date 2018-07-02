#ifndef _SIRIUS_INTF_H_
#define _SIRIUS_INTF_H_

#include "SiriusIntfData.h"

typedef int32_t (*RequestCbFunc)(RequestType type, int32_t id, void *header, uint8_t *dat);
typedef int32_t (*EventCbFunc)(int32_t event, int32_t arg1, int32_t arg2);

class SiriusIntf {
public:
    virtual int32_t request(RequestType type) = 0;
    virtual int32_t abort(RequestType type) = 0;
    virtual int32_t enqueueBuf(RequestType type, int32_t id) = 0;
    virtual int32_t setCallback(RequestCbFunc requestCb, EventCbFunc eventCb) = 0;

public:
    virtual ~SiriusIntf() {}
};

#endif
