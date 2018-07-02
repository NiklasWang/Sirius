#ifndef _ION_BUFFER_MANAGER_H__
#define _ION_BUFFER_MANAGER_H__

#include <linux/msm_ion.h>

#include "common.h"

namespace sirius {

#define ION_ALLOC_ALIGN_SIZE   4096
#define ION_ALLOC_FLAGS        ION_FLAG_CACHED
#define ION_ALLOC_HEAP_ID_MASK (0x1 << ION_IOMMU_HEAP_ID)

class IonBufferMgr :
    public noncopyable  {
public:
    int32_t allocate(void **buf, int32_t len);
    int32_t allocate(void **buf, int32_t len, int32_t *fd);
    int32_t clean(void *buf);
    int32_t flush(void *buf);
    int32_t invalidate(void *buf);
    int32_t import(void **buf, int32_t fd, int32_t len);
    int32_t release(void *buf);
    void clear_all();

public:
    IonBufferMgr();
    virtual ~IonBufferMgr();
    int32_t init();
    int32_t deinit();

private:
    struct Buffer {
        int32_t mFd;
        void   *mPtr;
        uint32_t mLen;
        ion_user_handle_t mIonHandle;
    };

private:
    int32_t allocate(Buffer *buf, int32_t len);
    int32_t import(Buffer *buf, int32_t fd, int32_t len);
    int32_t release(Buffer *buf);
    int32_t release_remove(Buffer *buf);
    int32_t cacheIoctl(void *buf, uint32_t cmd);
    int32_t cacheIoctl(Buffer *buf, uint32_t cmd);
    Buffer *findBuf(void *buf);

private:
    int32_t mIonFd;
    List<Buffer> mBuffers;
    RWLock       mBufLock;
    ModuleType   mModule;
};

};

#endif

