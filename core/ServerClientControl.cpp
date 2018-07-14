#include "ServerClientControl.h"

namespace sirius {

#define CHECK_MEM_AVAILABLE() \
    ({ \
        int32_t __rc = NO_ERROR; \
        if (ISNULL(mCtl)) { \
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

ServerClientControl::ServerClientControl() :
    mModule(MODULE_SERVER_CLIENT_CONTROL),
    mCtl(NULL),
    mReady(false)
{
}

ServerClientControl::~ServerClientControl()
{
}

int32_t ServerClientControl::setRequest(RequestType type, bool required)
{
    int32_t rc = CHECK_MEM_AVAILABLE();

    if (SUCCEED(rc)) {
        mCtl.request[getType(type)].requested = required;
    }

    return rc;
}

bool ServerClientControl::requested(RequestType type)
{
    int32_t rc = CHECK_MEM_AVAILABLE();
    bool result = false;

    if (SUCCEED(rc)) {
        result = mCtl.request[getType(type)].requested;
    }

    return result;
}

int32_t ServerClientControl::getUsedMem(
    RequestType type, int32_t *fd)
{
    int32_t rc = CHECK_MEM_AVAILABLE();
    RequestMemory *mem = mCtl.request[getType(type)].mems;
    int32_t index = -1;

    if (SUCCEED(rc)) {
        for (int32_t i = 0; i < mCtl.request[getType(type)].memNum; i++) {
            pthread_mutex_lock(&mem[i].l);
            if (mem[i].stat == MEMORY_STAT_USED && !mem[i].writting) {
                index = i;
                mem[i].writting = true;
            }
            pthread_mutex_unlock(&mem[i].l);
            if (index != -1) {
                break;
            }
        }
    }

    if (SUCCEED(rc)) {
        if (index == -1) {
            *fd = -1;
            rc = NOT_FOUND;
        } else {
            *fd = mCtl.request[getType(type)].mems[index].fd;
        }
    }

    return rc;
}

int32_t ServerClientControl::findClientMemory(
    RequestType type, int32_t fd, RequestMemory **mem)
{
    RequestType requestType = getType(type);
    RequestMemory *mems = mCtl.request[requestType].mems;
    bool found = false;

    *mem = NULL;
    for (int32_t i = 0; i < mCtl.request[requestType].memNum; i++) {
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
        mem->writting = false;
        mem->ts = currentUs();
    }

    return rc;
}

int32_t ServerClientControl::getMemStatus(RequestType type, int32_t fd, bool *fresh)
{
    int32_t rc = CHECK_MEM_AVAILABLE();
    RequestType requestType = getType(type);
    RequestMemory *mem = NULL;

    *fresh = USED_MEMORY;
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
        *fresh = mem->stat == MEMORY_STAT_FRESH ? FRESH_MEMORY : USED_MEMORY;
    }

    return rc;
}

int32_t ServerClientControl::setMemSize(RequestType type, int32_t size)
{
    int32_t rc = CHECK_MEM_AVAILABLE();

    if (SUCCEED(rc)) {
        mCtl.request[getType(type)].size = size;
    }

    return rc;
}

int32_t ServerClientControl::getMemSize(RequestType type, int32_t *size)
{
    int32_t rc = CHECK_MEM_AVAILABLE();

    if (SUCCEED(rc)) {
        *size = mCtl.request[getType(type)].size;
    }

    return rc;
}

int32_t ServerClientControl::addMemory(
    RequestType type, int32_t clientfd, bool fresh)
{
    int32_t rc = NOT_FOUND;
    RequestType requestType = getType(type);
    RequestMemory *mems = mCtl.request[requestType].mems;

    for (int32_t i = 0; i < REQUEST_HANDLER_MAX_MEMORY_NUM; i++) {
        if (mems[i].fd == -1) {
            mems[i].fd = clientfd;
            mems[i].stat = fresh ? MEMORY_STAT_FRESH : MEMORY_STAT_USED;
            mems[i].ts = currentUs();
            pthread_mutex_init(&mems[i].l, NULL);
            mems[i].writting = false;
            rc = NO_ERROR;
            break;
        }
    }

    if (SUCCEED(rc)) {
        mCtl.request[requestType].memNum++;
    }

    return rc;
}

int32_t ServerClientControl::getHeader(Header &header)
{
    header = mCtl.header;
    return NO_ERROR;
}

int32_t ServerClientControl::setHeader(Header &header)
{
    mCtl.header = header;
    return NO_ERROR;
}

int32_t ServerClientControl::getTotoalSize()
{
    return sizeof(ControlMemory);
}

int32_t ServerClientControl::init(void *mem, int32_t size, bool init)
{
    int32_t rc = NO_ERROR;
    ASSERT_LOG(mModule, NOTNULL(mem), "Memory must not be NULL");

    if (size < getTotoalSize()) {
        LOGE(mModule, "Invalid memory size, %d", size);
        rc = NO_MEMORY;
    } else {
        mCtl = static_cast<ControlMemory *>(mem);
    }

    if (SUCCEED(rc) && init) {
        memset(&mCtl.previewNV21Header,  0, sizeof(PreviewNV21Header));
        memset(&mCtl.pictureNV21Header,  0, sizeof(PictureNV21Header));
        memset(&mCtl.pictureBayerHeader, 0, sizeof(PictureBayerHeader));
        for (int32_t i = 0; i < REQUEST_TYPE_MAX_INVALID; i++) {
            mCtl.request[i].requested = false;
            mCtl.request[i].memNum = 0;
            mCtl.request[i].size = 0;
            for (int32_t j = 0; j < REQUEST_HANDLER_MAX_MEMORY_NUM; j++) {
                mCtl.request[i].mems[j].fd = -1;
                mCtl.request[i].mems[j].stat = MEMORY_STAT_USED;
                mCtl.request[i].mems[j].ts = 0;
                pthread_mutex_init(&mCtl.request[i].mems[j].l, NULL);
                mCtl.request[i].mems[j].writting = false;
            }
        }
    }

    if (SUCCEED(rc)) {
        mReady = true;
    }

    return rc;
}

bool ServerClientControl::ready()
{
    return mReady;
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

