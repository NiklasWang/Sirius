#ifndef _BUFFER_MANAGER_H__
#define _BUFFER_MANAGER_H__

namespace sirius {

class BufferMgrIntf;

class BufferMgr :
    public BufferMgrIntf,
    public noncopyable  {
public:
    int32_t allocate(void **buf, int32_t len) override;
    int32_t allocate(void **buf, int32_t len, int32_t *fd) override;
    int32_t clean(void *buf) override;
    int32_t flush(void *buf) override;
    int32_t import(void **buf, int32_t fd, int32_t len) override;
    int32_t release(void *buf) override;
    void clear_all() override;

public:
    int32_t init() override;
    int32_t deinit() override;
    BufferMgr();
    virtual ~BufferMgr();

private:
    BufferMgrIntf *mMgr;
};

};

#endif

#endif


