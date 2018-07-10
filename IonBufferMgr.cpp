#include <sys/mman.h>

#include "IonBufferMgr.h"
#include "ion.h"

namespace sirius {

#define ION_ALLOC_ALIGN_SIZE   4096
#define ION_ALLOC_FLAGS        ION_FLAG_CACHED
#define ION_ALLOC_HEAP_ID_MASK (0x1 << ION_IOMMU_HEAP_ID)

int32_t IonBufferMgr::mIonFd = -1;

pthread_mutex_t IonBufferMgr::mIonFdLocker = PTHREAD_MUTEX_INITIALIZER;

IonBufferMgr::IonBufferMgr() :
    mModule(MODULE_ION_BUF_MANAGER)
{
}

IonBufferMgr::~IonBufferMgr()
{
    deinit();
}

int32_t IonBufferMgr::init()
{
    int32_t rc = NO_ERROR;

    if (mIonFd < 0) {
        pthread_mutex_lock(&mIonFdLocker);
        if (mIonFd < 0) {
            mIonFd = open("/dev/ion", O_RDONLY);
            if (mIonFd < 0) {
                LOGE(mModule, "Failed to open /dev/ion, %s", strerror(errno));
                rc = SYS_ERROR;
            }
        }
        pthread_mutex_unlock(&mIonFdLocker);
    }

    return rc;
}

int32_t IonBufferMgr::deinit()
{
    if (mIonFd > 0) {
        pthread_mutex_lock(&mIonFdLocker);
        if (mIonFd > 0) {
            close(mIonFd);
            mIonFd = -1;
        }
        pthread_mutex_unlock(&mIonFdLocker);
    }

    if (mBuffers.size()) {
        clear_all();
    }

    return NO_ERROR;
}

int32_t IonBufferMgr::allocate(void **buf, int32_t len)
{
    int32_t fd = -1;
    return allocate(buf, len, &fd);
}

int32_t IonBufferMgr::allocate(void **buf, int32_t len, int32_t *fd)
{
    int32_t rc = NO_ERROR;
    Buffer buffer;
    int32_t aligned = len;

    if (SUCCEED(rc)) {
        aligned = align_len_to_size(len, ION_ALLOC_ALIGN_SIZE);
        rc = allocate(&buffer, aligned);
        if (SUCCEED(rc)) {
            *buf = buffer.mPtr;
            *fd  = buffer.mFd;
            RWLock::AutoWLock l(mBufLock);
            mBuffers.push_back(buffer);
        }
    }

    return rc;
}

int32_t IonBufferMgr::allocate(Buffer *buf, int32_t len)
{
    int32_t rc = NO_ERROR;
    int32_t fd = -1;
    void *addr = NULL;
    ion_user_handle_t handle;

    if (SUCCEED(rc)) {
        rc = ion_alloc(mIonFd, len, ION_ALLOC_ALIGN_SIZE,
            ION_ALLOC_HEAP_ID_MASK, ION_ALLOC_FLAGS, &handle);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to alloc %d bytes from ion", len);
        }
    }

    if (SUCCEED(rc)) {
        rc = ion_share(mIonFd, handle, &fd);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to share, free allocated ion mem, %d", rc);
            rc = ion_free(mIonFd, handle);
            if (!SUCCEED(rc)) {
                LOGE(mModule, "Failed to free ion memory, %d", rc);
            }
            rc = SYS_ERROR;
        }
    }

    if (SUCCEED(rc)) {
        addr = mmap(NULL, len, PROT_READ | PROT_WRITE,
            MAP_SHARED, fd, 0);
        if (addr == MAP_FAILED) {
            LOGE(mModule, "Failed to mmap on fd %d len %d", fd, len);
            rc = ion_free(mIonFd, handle);
            if (!SUCCEED(rc)) {
                LOGE(mModule, "Failed to free ion memory, %d", rc);
            }
            rc = SYS_ERROR;
        }
    }

    if (SUCCEED(rc)) {
        buf->mPtr = addr;
        buf->mLen = len;
        buf->mFd = fd;
        buf->mIonHandle = handle;
    }

    return rc;
}

int32_t IonBufferMgr::clean(void *buf)
{
    return cacheIoctl(buf, ION_IOC_CLEAN_CACHES);
}

int32_t IonBufferMgr::flush(void *buf)
{
    Buffer *buffer = findBuf(buf);
    msync(buffer->mPtr, buffer->mLen, MS_SYNC);
    return cacheIoctl(buffer->mPtr, ION_IOC_CLEAN_INV_CACHES);
}

int32_t IonBufferMgr::invalidate(void *buf)
{
    return cacheIoctl(buf, ION_IOC_INV_CACHES);
}

int32_t IonBufferMgr::cacheIoctl(void *buf, uint32_t cmd)
{
    int32_t rc = NO_ERROR;
    Buffer *buffer = findBuf(buf);

    if (ISNULL(buffer)) {
        LOGE(mModule, "Failed to find memory.");
        rc = NO_MEMORY;
    }

    if (SUCCEED(rc)) {
        rc = cacheIoctl(buffer, cmd);
    }

    return rc;
}

int32_t IonBufferMgr::cacheIoctl(Buffer *buf, uint32_t cmd)
{
    int32_t rc = NO_ERROR;
    struct ion_flush_data flushData;
    struct ion_custom_data customData;
    memset(&flushData, 0x0, sizeof(flushData));
    memset(&customData, 0x0, sizeof(customData));

    flushData.vaddr = buf->mPtr;
    flushData.fd = buf->mFd;
    flushData.handle = buf->mIonHandle;
    flushData.length = buf->mLen;

    customData.cmd = cmd;
    customData.arg = (unsigned long)&flushData;

    rc = ioctl(mIonFd, ION_IOC_CUSTOM, &customData);
    if (!SUCCEED(rc)) {
       LOGE(mModule, "Failed cache ioctl, %s", strerror(errno));
    }

    return rc;
}

IonBufferMgr::Buffer *IonBufferMgr::findBuf(void *buf)
{
    Buffer *result = NULL;

    RWLock::AutoRLock l(mBufLock);
    for (auto &itr : mBuffers) {
        if (buf == itr.mPtr) {
            result = &itr;
            break;
        }
    }

    return result;
}

int32_t IonBufferMgr::import(void **buf, int32_t fd, int32_t len)
{
    int32_t rc = NO_ERROR;
    Buffer buffer;

    if (SUCCEED(rc)) {
        rc = import(&buffer, fd, len);
        if (SUCCEED(rc)) {
            *buf = buffer.mPtr;
            RWLock::AutoWLock l(mBufLock);
            mBuffers.push_back(buffer);
        }
    }

    return rc;
}

int32_t IonBufferMgr::import(Buffer *buf, int32_t fd, int32_t len)
{
    int32_t rc = NO_ERROR;
    void *addr = NULL;
    ion_user_handle_t handle;

    if (SUCCEED(rc)) {
        rc = ion_import(mIonFd, fd, &handle);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to import ion memory with fd %d", fd);
        }
    }

    if (SUCCEED(rc)) {
        addr = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (addr == MAP_FAILED) {
            LOGE(mModule, "Failed to mmap with fd %d", fd);
            rc = SYS_ERROR;
        }
    }

    if (SUCCEED(rc)) {
        buf->mPtr = addr;
        buf->mLen = len;
        buf->mFd = fd;
        buf->mIonHandle = handle;
    }

    return rc;
}

int32_t IonBufferMgr::release(void *buf)
{
    int32_t rc = NO_ERROR;
    Buffer *buffer = findBuf(buf);

    if (ISNULL(buffer)) {
        LOGE(mModule, "Failed to find memory.");
        rc = NO_MEMORY;
    }

    if (SUCCEED(rc)) {
        rc = release_remove(buffer);
    }

    return rc;
}

int32_t IonBufferMgr::release(Buffer *buf)
{
    int32_t rc = NO_ERROR;

    if (SUCCEED(rc)) {
        if (!ISNULL(buf->mPtr)) {
            munmap(buf->mPtr, buf->mLen);
            close(buf->mFd);
        } else {
            rc = PARAM_INVALID;
        }

    }

    if (SUCCEED(rc)) {
        rc = ion_free(mIonFd, buf->mIonHandle);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to free ion mem, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        buf->mPtr = NULL;
        buf->mLen = 0;
        buf->mFd = -1;
        buf->mIonHandle = -1;
    }

    return rc;
}

int32_t IonBufferMgr::release_remove(Buffer *buf)
{
    int32_t rc = NO_ERROR;
    bool found = false;
    Buffer buf_copied = *buf;

    if (SUCCEED(rc)) {
        rc = release(&buf_copied);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to release buf");
        }
    }

    if (SUCCEED(rc)) {
        RWLock::AutoWLock l(mBufLock);
        auto iter = mBuffers.begin();
        while (iter != mBuffers.end()) {
            if (iter->mIonHandle == buf->mIonHandle) {
                iter = mBuffers.erase(iter);
                found = true;
            } else {
                iter++;
            }
        }
    }

    if (SUCCEED(rc)) {
        if (!found) {
            LOGE(mModule, "Buffer not found in container");
            rc = NOT_FOUND;
        }
    }

    return rc;
}

void IonBufferMgr::clear_all()
{
    RWLock::AutoWLock l(mBufLock);
    auto iter = mBuffers.begin();
    while (iter != mBuffers.end()) {
        release(iter->mPtr);
        mBuffers.erase(iter);
        iter = mBuffers.begin();
    }
}

};

