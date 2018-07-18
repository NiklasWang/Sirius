#ifndef _HANDLER_OPERATIONS_INTERFACE_H_
#define _HANDLER_OPERATIONS_INTERFACE_H_

#include <stdint.h>

#include "SiriusIntf.h"

namespace sirius {

#define FRESH_MEMORY    1
#define USED_MEMORY     0
#define ENABLE_REQUEST  1
#define DISABLE_REQUEST 0

class HandlerOpsIntf {
public:
    virtual int32_t send(RequestType type, int32_t id, void *header, void *dat) = 0;
    virtual int32_t send(int32_t event, int32_t arg1, int32_t arg2) = 0;
    virtual int32_t allocateBuf(void **buf, int32_t len, int32_t *fd) = 0;
    virtual int32_t releaseBuf(void *buf) = 0;
    virtual int32_t setMemStatus(RequestType type, int32_t fd, bool fresh = USED_MEMORY) = 0;
    virtual int32_t getMemStatus(RequestType type, int32_t fd, bool *fresh) = 0;
    virtual int32_t setMemSize(RequestType type, int32_t size) = 0;
    virtual int32_t getMemSize(RequestType type, int32_t *size) = 0;
    virtual int32_t addMemory(RequestType type, int32_t clientfd, bool fresh = USED_MEMORY) = 0;
    virtual int32_t setRequestedMark(RequestType type, bool enable = DISABLE_REQUEST) = 0;
    virtual int32_t getHeader(Header &header) = 0;

public:
    virtual ~HandlerOpsIntf() {}
};

};

#endif
