#ifndef _BUFFER_MANAGER_INTF_H_
#define _BUFFER_MANAGER_INTF_H_

#include <stdint.h>

class BufferMgrIntf
{
public:    
    virtual int32_t init() = 0;
    virtual int32_t deinit() = 0;

    virtual int32_t allocate(void **buf, int32_t len) = 0;
    virtual int32_t allocate(void **buf, int32_t len, int32_t *fd) = 0;
    virtual int32_t clean(void *buf) = 0;
    virtual int32_t flush(void *buf) = 0;
    virtual int32_t import(void **buf, int32_t fd, int32_t len) = 0;
    virtual int32_t release(void *buf) = 0;
    virtual void clear_all() = 0;

public:
    virtual ~BufferMgrIntf() {}
};

#endif

