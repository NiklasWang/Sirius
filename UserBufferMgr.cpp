#include "modules.h"
#include "CQueue.h"
#include "UserBufferMgr.h"

namespace sirius {

UserBufferMgr::UserBufferMgr() :
    mId(0),
    mModule(MODULE_USER_BUF_MANAGER),
    mQ(NULL)
{
}

#define CONSTRUCT_QUEUE_ONCE() \
    ({ \
        int32_t __rc = NO_ERROR; \
        if (ISNULL(mQ)) { \
            mQ = new CQueue(); \
            if (ISNULL(mQ)) { \
                LOGE(mModule, "Failed to create buffer queue"); \
                __rc = NOT_INITED; \
            } \
        } \
        __rc; \
    })

int32_t UserBufferMgr::enqueue(void *buffer, int32_t size)
{
    int32_t rc = CONSTRUCT_QUEUE_ONCE();
    Buffer *buf = NULL;

    if (SUCCEED(rc)) {
        buf = new Buffer();
        if (ISNULL(buf)) {
            LOGE(mModule, "Failed to new buffer object");
            rc = NO_MEMORY;
        }
    }

    if (SUCCEED(rc)) {
        buf->id = mId++;
        buf->buf = buffer;
        buf->size = size;
        rc = mQ->enqueue(buf) ? NO_ERROR : NOT_INITED;
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to enqueue buf");
        }
    }

    return rc;
}

int32_t UserBufferMgr::dequeue(void **buf, int32_t *size)
{
    int32_t rc = CONSTRUCT_QUEUE_ONCE();
    Buffer *buffer = NULL;

    if (SUCCEED(rc)) {
        buffer = static_cast<Buffer *>(mQ->dequeue());
        if (ISNULL(buffer)) {
            LOGI(mModule, "No available buffer, continue");
            rc = NOT_FOUND;
        }
    }

    if (SUCCEED(rc)) {
        *buf = SUCCEED(rc) ? buffer->buf : NULL;
        *size = SUCCEED(rc) ? buffer->size : 0;
    }

    if (SUCCEED(rc)) {
        delete buffer;
        buffer = NULL;
    }

    return rc;
}

int32_t UserBufferMgr::size()
{
    return mQ->size();
}

bool UserBufferMgr::isEmpty()
{
    return mQ->isEmpty();
}

void UserBufferMgr::flush()
{
    mQ->flush();
    mQ->init();
}

};

