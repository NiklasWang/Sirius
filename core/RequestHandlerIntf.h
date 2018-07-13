#ifndef _REQUEST_HANDLER_INTF_H_
#define _REQUEST_HANDLER_INTF_H_

#include "SiriusIntfData.h"

class RequestHandlerIntf {
public:

    virtual int32_t enqueue(int32_t id) = 0;
    virtual int32_t abort() = 0;
    virtual int32_t setSocketFd(int32_t fd) = 0;
    virtual int32_t onClientReady() = 0;

    virtual RequestType getType() = 0;
    virtual const char *getName() = 0;

    virtual int32_t construct() = 0;
    virtual int32_t destruct() = 0;

public:
    virtual ~RequestHandlerIntf() {}
};

#endif
