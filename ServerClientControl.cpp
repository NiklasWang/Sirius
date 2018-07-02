#include "ServerClientControl.h"

namespace sirius {

#define CHECK_MEM_AVAILABLE() \
    ({ \
        int32_t __rc = NO_ERROR; \
        if (ISNULL(mMem)) { \
            LOGD(mModule, "Memory not set."); \
            __rc = NOT_INITED; \
        } \
        __rc; \
    })

RequestType getType(RequestType type)
{
    return (type < 0 || type > REQUEST_TYPE_MAX_INVALID) ?
        REQUEST_TYPE_MAX_INVALID : type;
}

void ServerClientControl::SemaphoreEx::init(int32_t n)
{
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
    cnt = n;
}

ServerClientControl::ServerClientControl() :
    mModule(MODULE_SERVER_CLIENT_CONTROL),
    mMem(NULL)
{
}

ServerClientControl::~ServerClientControl()
{
}

int32_t ServerClientControl::setRequest(RequestType type, bool required)
{
    int32_t rc = CHECK_MEM_AVAILABLE();
    bool prev = false;

    if (SUCCEED(rc)) {
        prev = mMem[getType(type)].requests;
        mMem[getType(type)].requests = required;
    }

    if (SUCCEED(rc)) {
        if (prev != required) {
            mMem[getType(type)].sem.init();
        }
    }

    return rc;
}

bool ServerClientControl::requests(RequestType type)
{
    int32_t rc = CHECK_MEM_AVAILABLE();
    bool result = false;

    if (SUCCEED(rc)) {
        result = mMem[getType(type)].requests;
    }

    return result;
}

int32_t ServerClientControl::getFirstFreshMemLock(
    RequestType type, int32_t *fd)
{
    int32_t rc = CHECK_MEM_AVAILABLE();
    RequestType requestType = getType(type);
    RequestMemory *mem = mMem[requestType].mems;
    int64_t ts = 2^63 - 1;
    int32_t index = -1;
    int32_t lastIndex = -1;

    if (SUCCEED(rc)) {
        for (int32_t i = 0; i < mMem[requestType].memNum; i++) {
            pthread_mutex_lock(&mem[i].l);
            if (ts > mem[i].ts &&
                mem[i].stat == MEMORY_STAT_FRESH) {
                ts = mem[i].ts;
                index = i;
                if (lastIndex != -1) {
                    pthread_mutex_unlock(&mem[lastIndex].l);
                }
                lastIndex = index;
            } else {
                pthread_mutex_unlock(&mem[i].l);
            }
        }
    }

    if (SUCCEED(rc)) {
        if (index == -1) {
            *fd = -1;
        } else {
            *fd = mMem[requestType].mems[index].fd;
        }
    }

    return rc;
}

int32_t ServerClientControl::getUsedMemLock(
    RequestType type, int32_t *fd)
{
    int32_t rc = CHECK_MEM_AVAILABLE();
    RequestMemory *mem = mMem[getType(type)].mems;
    int32_t index = -1;

    if (SUCCEED(rc)) {
        for (int32_t i = 0; i < mMem[getType(type)].memNum; i++) {
            pthread_mutex_lock(&mem[i].l);
            if (mem[i].stat == MEMORY_STAT_USED) {
                index = i;
                break;
            }
            pthread_mutex_unlock(&mem[i].l);
        }
    }

    if (SUCCEED(rc)) {
        if (index == -1) {
            *fd = -1;
        } else {
            *fd = mMem[getType(type)].mems[index].fd;
        }
    }

    return rc;
}

int32_t ServerClientControl::findClientMemory(
    RequestType type, int32_t fd, RequestMemory **mem)
{
    RequestType requestType = getType(type);
    RequestMemory *mems = mMem[requestType].mems;
    bool found = false;

    *mem = NULL;
    for (int32_t i = 0; i < mMem[requestType].memNum; i++) {
        if (mems[i].fd == fd) {
            *mem = &mems[i];
            found = true;
            break;
        }
    }

    return !found ? NOT_FOUND : NO_ERROR;
}

int32_t ServerClientControl::setMemStatus(
    RequestType type, int32_t fd, bool fresh)
{
    int32_t rc = CHECK_MEM_AVAILABLE();
    RequestType requestType = getType(type);
    RequestMemory *mem = NULL;

    if (SUCCEED(rc)) {
        rc = findClientMemory(requestType, fd, &mem);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to find client memory, %d", rc);
        }
        if (ISNULL(mem)) {
            LOGE(mModule, "Not found client memory fd %d, %d", fd, rc);
            rc = NOT_FOUND;
        }
    }

    if (SUCCEED(rc)) {
        mem->stat = fresh ? MEMORY_STAT_FRESH : MEMORY_STAT_USED;
        mem->ts = currentUs();
    }

    return rc;
}

int32_t ServerClientControl::setMemSize(RequestType type, int32_t size)
{
    int32_t rc = CHECK_MEM_AVAILABLE();

    if (SUCCEED(rc)) {
        mMem[getType(type)].size = size;
    }

    return rc;
}

int32_t ServerClientControl::getMemSize(RequestType type, int32_t *size)
{
    int32_t rc = CHECK_MEM_AVAILABLE();

    if (SUCCEED(rc)) {
        *size = mMem[getType(type)].size;
    }

    return rc;
}

int32_t ServerClientControl::lockMemory(RequestType type, int32_t fd)
{
    int32_t rc = CHECK_MEM_AVAILABLE();
    RequestType requestType = getType(type);
    RequestMemory *mem = NULL;

    if (SUCCEED(rc)) {
        rc = findClientMemory(requestType, fd, &mem);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to find client memory, %d", rc);
        }
        if (ISNULL(mem)) {
            LOGE(mModule, "Not found client memory fd %d, %d", fd, rc);
            rc = NOT_FOUND;
        }
    }

    if (SUCCEED(rc)) {
        pthread_mutex_lock(&mem->l);
    }

    return rc;
}

int32_t ServerClientControl::addMemory(
    RequestType type, int32_t clientfd, bool fresh)
{
    int32_t rc = NOT_FOUND;
    RequestType requestType = getType(type);
    RequestMemory *mems = mMem[requestType].mems;

    for (int32_t i = 0; i < REQUEST_HANDLER_MAX_MEMORY_NUM; i++) {
        if (mems[i].fd == -1) {
            mems[i].fd = clientfd;
            mems[i].stat = fresh ? MEMORY_STAT_FRESH : MEMORY_STAT_USED;
            mems[i].ts = currentUs();
            pthread_mutex_init(&mems[i].l, NULL);
            rc = NO_ERROR;
            break;
        }
    }

    if (SUCCEED(rc)) {
        mMem[requestType].memNum++;
    }

    return rc;
}


int32_t ServerClientControl::unlockMemory(RequestType type, int32_t fd)
{
    int32_t rc = CHECK_MEM_AVAILABLE();
    RequestType requestType = getType(type);
    RequestMemory *mem = NULL;

    if (SUCCEED(rc)) {
        rc = findClientMemory(requestType, fd, &mem);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to find client memory, %d", rc);
        }
        if (ISNULL(mem)) {
            LOGE(mModule, "Not found client memory fd %d, %d", fd, rc);
            rc = NOT_FOUND;
        }
    }

    if (SUCCEED(rc)) {
        pthread_mutex_unlock(&mem->l);
    }

    return rc;
}

Semaphore *ServerClientControl::getSemaphore(RequestType type)
{
    int32_t rc = CHECK_MEM_AVAILABLE();
    Semaphore *result = NULL;

    if (SUCCEED(rc)) {
        result = &mMem[getType(type)].sem;
    }

    return result;
}

int32_t ServerClientControl::getTotoalSize()
{
    return sizeof(MemoryBlock) * REQUEST_TYPE_MAX_INVALID;
}

int32_t ServerClientControl::setMemory(void *mem, int32_t size, bool init)
{
    int32_t rc = NO_ERROR;
    ASSERT_LOG(mModule, NOTNULL(mem), "Memory must not be NULL");

    if (size < getTotoalSize()) {
        LOGE(mModule, "Invalid memory size, %d", size);
        rc = NO_MEMORY;
    }

    if (SUCCEED(rc)) {
        mMem = static_cast<MemoryBlock *>(mem);
    }

    if (SUCCEED(rc) && init) {
        for (int32_t i = 0; i < REQUEST_TYPE_MAX_INVALID; i++) {
            mMem[i].requests = false;
            mMem[i].memNum = 0;
            mMem[i].size = 0;
            mMem[i].sem.init();
            for (int32_t j = 0; j < REQUEST_HANDLER_MAX_MEMORY_NUM; j++) {
                mMem[i].mems[j].fd = -1;
                mMem[i].mems[j].stat = MEMORY_STAT_USED;
                mMem[i].mems[j].ts = 0;
                pthread_mutex_init(&mMem[i].mems[j].l, NULL);
            }
        }
    }

    return rc;
}

int64_t ServerClientControl::currentUs()
{
    int64_t t = 0;

    struct timeval tv;
    gettimeofday(&tv, NULL);
    t = (int64_t)tv.tv_usec + tv.tv_sec * 1000000ll;

    return t;
}

};

