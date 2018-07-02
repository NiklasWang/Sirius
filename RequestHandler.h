#ifndef _REQUEST_HANDLER_H_
#define _REQUEST_HANDLER_H_

#include "common.h"
#include "RunOnceThread.h"
#include "SiriusIntf.h"
#include "HandlerOpsIntf.h"
#include "UserBufferMgr.h"

namespace sirius {

class RequestHandler :
    public RunOnceFunc {
public:
    RequestType getType();
    const char *getName();

    int32_t enqueue(void *buf, int32_t size);
    int32_t dequeue(void **buf, int32_t *size);

    int32_t wait();
    int32_t abort();

    int32_t getDataSize(); // size of buffer to be enqueue

    int32_t onClientReady();

public:
    int32_t runOnceFunc(void *in, void *out) override;
    int32_t onOnceFuncFinished(int32_t rc) override;
    int32_t abortOnceFunc() override;

public:
    RequestHandler(HandlerOpsIntf *ops,
        RequestType type, const char *name, uint32_t memNum);
    virtual ~RequestHandler();
    int32_t construct();
    int32_t destruct();

protected:
    virtual int32_t getPrivateDataSize() = 0;
    virtual int32_t getRequestDataSize() = 0;
    virtual int32_t copyPrivateData(void *dst, void *src, int32_t *size) = 0;

private:
    int32_t allocMemAndShare();
    int32_t allocMem();
    int32_t releaseMem();
    int32_t shareSingleMem(int32_t fd);
    int32_t shareMem();
    int32_t processClientUpdate();
    int32_t copyData(void *dst, void *src, int32_t len);
    int32_t recordMemoryPair(int32_t serverfd, int32_t clientfd);
    int32_t convertToClientFd(char *msg, int32_t *clientfd);

    enum MemStatus {
        MEM_STAT_USED,
        MEM_STAT_FRESH,
    };

    struct MemoryInfo {
        void   *buf;
        int32_t  size;
        int32_t serverfd;
        int32_t clientfd;
    };

private:
    int32_t getFirstFreshMemLock(MemoryInfo **mem);
    int32_t copyToUserBuf(MemoryInfo *mem,
        void *userbuf = NULL, int32_t usersize = 0);

    class RunOnce :
        public RunOnceThread {
    public:
        int32_t run(RunOnceFunc *func, void *in, void *out);
        int32_t exit();
        bool    isRuning();
    };

private:
    bool            mConstructed;
    ModuleType      mModule;
    const char     *mName;
    RequestType     mType;
    HandlerOpsIntf *mOps;
    bool            mMemShared;
    uint32_t        mMemNum;
    MemoryInfo     *mMem;
    UserBufferMgr   mBufMgr;
    RunOnce        *mRunOnce;
    bool            mAbortOnce;
    static pthread_mutex_t mSocketL; // Lock for all inherited objects
};

};

#endif

