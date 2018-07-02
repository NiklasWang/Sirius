#ifndef _USER_BUFFER_MANAGER_H_
#define _USER_BUFFER_MANAGER_H_

#include "common.h"

namespace sirius {
    class CQueue;
};

namespace sirius {

class UserBufferMgr :
    public noncopyable {
public:
    int32_t enqueue(void *buf, int32_t size);
    int32_t dequeue(void **buf, int32_t *size);
    int32_t size();
    bool    isEmpty();
    void    flush();

public:
    UserBufferMgr();
    virtual ~UserBufferMgr() {}

private:
    struct Buffer {
        uint32_t id;
        void *buf;
        int32_t size;
    };

private:
    uint32_t   mId;
    ModuleType mModule;
    CQueue    *mQ;
};

};

#endif
