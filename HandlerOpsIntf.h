#ifndef _HANDLER_OPERATIONS_INTERFACE_H_
#define _HANDLER_OPERATIONS_INTERFACE_H_

#include <stdint.h>

#include "SiriusIntf.h"
#include "Semaphore.h"

namespace sirius {

#define FRESH_MEMORY    1
#define USED_MEMORY     0
#define ENABLE_REQUEST  1
#define DISABLE_REQUEST 0

class HandlerOpsIntf {
public:
    virtual int32_t sendCallback(RequestType type, int32_t id, void *header, uint8_t *dat) = 0;
    virtual int32_t allocateIon(void **buf, int32_t len, int32_t *fd) = 0;
    virtual int32_t releaseIon(void *buf) = 0;
    virtual int32_t getUsedMemLock(RequestType type, int32_t *fd) = 0;
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
