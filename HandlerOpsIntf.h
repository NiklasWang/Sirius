#ifndef _HANDLER_OPERATIONS_INTERFACE_H_
#define _HANDLER_OPERATIONS_INTERFACE_H_

#include <stdint.h>

#include "SiriusIntf.h"
#include "Semaphore.h"

namespace sirius {

class HandlerOpsIntf {
public:
    virtual int32_t sendCallback(RequestType type, void *data) = 0;
    virtual int32_t waitClientSem(RequestType type, Semaphore **sem) = 0;
    virtual int32_t sendClientFd(int32_t fd) = 0;
    virtual int32_t sendClientMsg(const char *data, int32_t len) = 0;
    virtual int32_t receiveClientMsg(char *data, int32_t maxlen) = 0;
    virtual int32_t allocateIon(void **buf, int32_t len, int32_t *fd) = 0;
    virtual int32_t releaseIon(void *buf) = 0;
    virtual int32_t getFirstFreshMemLock(RequestType type, int32_t *fd) = 0;
    virtual int32_t getUsedMemLock(RequestType type, int32_t *fd) = 0;
    virtual int32_t setMemStatus(RequestType type, int32_t fd, bool fresh = false) = 0;
    virtual int32_t setMemSize(RequestType type, int32_t size) = 0;
    virtual int32_t getMemSize(RequestType type, int32_t *size) = 0;
    virtual int32_t lockMemory(RequestType type, int32_t fd) = 0;
    virtual int32_t addMemory(RequestType type, int32_t clientfd, bool fresh = false) = 0;
    virtual int32_t unlockMemory(RequestType type, int32_t fd) = 0;
    virtual int32_t setRequestedMark(RequestType type, bool enable = false) = 0;
    virtual bool clientReady() = 0;

public:
    virtual ~HandlerOpsIntf() {}
};

};

#endif
