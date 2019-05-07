#include "BufferMgr.h"

#ifdef ENABLE_ION_BUFFER

#include "IonBufferMgr.h"

#define CONSTRUCT_IMPL() \
    ({ \
        int32_t __rc = NO_ERROR; \
        if (ISNULL(mMgr)) { \
            mMgr = new IonBufferMgr(); \
            if (ISNULL(mMgr)) { \
                LOGE(MODULE_BUF_MANAGER, "Failed to create ion buffer manager."); \
                __rc = NOT_INITED; \
            } \
        } \
        __rc; \
    })

#else
#error Not implemented buffer manager impl for this platform.
#endif

#define CHECK_VALID_IMPL() \
        ({ \
            int32_t __rc = NO_ERROR; \
            if (ISNULL(mMgr)) { \
                LOGD(MODULE_BUF_MANAGER, "Sirius impl not created"); \
                __rc = NOT_INITED; \
            } \
            __rc; \
        })

#define CONSTRUCT_IMPL_ONCE() \
    ({ \
        int32_t __rc = CHECK_VALID_IMPL(); \
        if (__rc == NOT_INITED) { \
            __rc = CONSTRUCT_IMPL(); \
        } \
        __rc; \
    })

namespace sirius {

BufferMgr::BufferMgr() :
    mMgr(NULL)
{
}

BufferMgr::~BufferMgr()
{
    if (NOTNULL(mMgr)) {
        SECURE_DELETE(mMgr);
    }
}

int32_t BufferMgr::init()
{
    int32_t rc = CONSTRUCT_IMPL_ONCE();
    return SUCCEED(rc) ? mMgr->init() : rc;
}

int32_t BufferMgr::deinit()
{
    int32_t rc = CONSTRUCT_IMPL_ONCE();
    return SUCCEED(rc) ? mMgr->deinit() : rc;
}

int32_t BufferMgr::allocate(void **buf, int32_t len)
{
    int32_t rc = CONSTRUCT_IMPL_ONCE();
    return SUCCEED(rc) ? mMgr->allocate(buf, len) : rc;
}

int32_t BufferMgr::allocate(void **buf, int32_t len, int32_t *fd)
{
    int32_t rc = CONSTRUCT_IMPL_ONCE();
    return SUCCEED(rc) ? mMgr->allocate(buf, len, fd) : rc;
}

int32_t BufferMgr::clean(void *buf)
{
    int32_t rc = CONSTRUCT_IMPL_ONCE();
    return SUCCEED(rc) ? mMgr->clean(buf) : rc;
}

int32_t BufferMgr::flush(void *buf)
{
    int32_t rc = CONSTRUCT_IMPL_ONCE();
    return SUCCEED(rc) ? mMgr->flush(buf) : rc;
}

int32_t BufferMgr::import(void **buf, int32_t fd, int32_t len)
{
    int32_t rc = CONSTRUCT_IMPL_ONCE();
    return SUCCEED(rc) ? mMgr->import(buf, fd, len) : rc;
}

int32_t BufferMgr::release(void *buf)
{
    int32_t rc = CONSTRUCT_IMPL_ONCE();
    return SUCCEED(rc) ? mMgr->release(buf) : rc;
}

int32_t BufferMgr::release_remove(void *buf)
{
    int32_t rc = CONSTRUCT_IMPL_ONCE();
    return SUCCEED(rc) ? mMgr->release_remove(buf) : rc;
}

void BufferMgr::clear_all()
{
    CONSTRUCT_IMPL_ONCE();
    mMgr->clear_all();
}

};

