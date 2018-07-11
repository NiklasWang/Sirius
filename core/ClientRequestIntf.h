#ifndef _CLIENT_REQUEST_INTF_H_
#define _CLIENT_REQUEST_INTF_H_

#include <stdint.h>

#include "SiriusIntf.h"

namespace sirius {

class ClientRequestIntf {
public:
    virtual int32_t sizeOfHeader() = 0;
    virtual int32_t copyHeader(void *dst, int32_t maxSize) = 0;
    virtual int32_t copyData(void *dst, int32_t maxSize) = 0;
    virtual RequestType type() = 0;
    virtual const char *name() = 0;

    virtual ~ClientRequestIntf() {}
};

};

#endif
