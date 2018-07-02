#ifndef _SERVER_CLIENT_CONTROL_H_
#define _SERVER_CLIENT_CONTROL_H_

#include "common.h"
#include "Semaphore.h"
#include "configuration.h"
#include "SiriusIntf.h"

namespace sirius {

class ServerClientControl :
    public noncopyable {
public:
    int32_t setRequest(RequestType type, bool required = false);
    bool requests(RequestType type);
    int32_t getFirstFreshMemLock(RequestType type, int32_t *fd);
    int32_t getUsedMemLock(RequestType type, int32_t *fd);
    int32_t setMemStatus(RequestType type, int32_t fd, bool fresh = false);
    int32_t setMemSize(RequestType type, int32_t size);
    int32_t getMemSize(RequestType type, int32_t *size);
    int32_t lockMemory(RequestType type, int32_t fd);
    int32_t addMemory(RequestType type, int32_t clientfd, bool fresh = false);
    int32_t unlockMemory(RequestType type, int32_t fd);
    Semaphore *getSemaphore(RequestType type);

public:
    int32_t getTotoalSize();
    int32_t setMemory(void *mem, int32_t size, bool init = false);

public:
    ServerClientControl();
    virtual ~ServerClientControl();

private:
    int64_t currentUs();
    struct RequestMemory;
    int32_t findClientMemory(RequestType type,
        int32_t fd, RequestMemory **mem);

private:
    class SemaphoreEx :
        public Semaphore {
    public:
        void init(int32_t n = 0);
        SemaphoreEx(int32_t n = 0) :
            Semaphore(n) {}
    };

    enum MemoryStatus {
        MEMORY_STAT_USED,
        MEMORY_STAT_FRESH,
    };

    struct RequestMemory {
        int32_t fd;
        MemoryStatus stat;
        int64_t ts;
        pthread_mutex_t l;
    };

    struct MemoryBlock {
        bool          requests;
        int32_t       memNum;
        int32_t       size;
        RequestMemory mems[REQUEST_HANDLER_MAX_MEMORY_NUM];
        SemaphoreEx   sem;
    };

private:
    ModuleType   mModule;
    MemoryBlock *mMem;
};

};

#endif
