#ifndef _REQUEST_HANDLER_H_
#define _REQUEST_HANDLER_H_

#include "common.h"
#include "RunOnceThread.h"
#include "SiriusIntf.h"
#include "HandlerOpsIntf.h"

namespace sirius {

class RequestHandler :
    public RunOnceFunc {
public:
    RequestType getType();
    const char *getName();

    int32_t enqueue(int32_t id);
    int32_t abort();
    int32_t setSocketFd(int32_t fd);
    int32_t onClientReady();

public:
    RequestHandler(HandlerOpsIntf *ops,
        RequestType type, const char *name, uint32_t memNum);
    virtual ~RequestHandler();
    int32_t construct();
    int32_t destruct();

protected:
    virtual int32_t getHeaderSize() = 0;
    virtual int32_t getDataSize() = 0;

private:
    virtual int32_t runOnceFunc(void *in, void *out) override;
    virtual int32_t onOnceFuncFinished(int32_t rc) override;
    virtual int32_t abortOnceFunc() override;

private:
    int32_t getExpectedBufferSize();
    int32_t allocMemAndShare();
    int32_t allocMem();
    int32_t releaseMem();
    int32_t shareSingleMem(int32_t fd);
    int32_t shareMem();
    int32_t onMemRefreshed(int32_t fd);
    int32_t recordMemoryPair(int32_t serverfd, int32_t clientfd);
    int32_t convertToClientFd(char *msg, const char *prefix, int32_t *clientfd);

private:
    struct MemoryInfo {
        int32_t id;
        void   *buf;
        int32_t size;
        int32_t serverfd;
        int32_t clientfd;
    };

    class RunOnce :
        public RunOnceThread {
    public:
        int32_t run(RunOnceFunc *func, void *in, void *out);
        int32_t exit();
        bool    isRuning();
    };

private:
    bool         mConstructed;
    ModuleType   mModule;
    const char  *mName;
    RequestType  mType;
    bool         mMemShared;
    uint32_t     mMemNum;
    MemoryInfo  *mMem;
    RunOnce     *mRunOnce;

protected:
    Header          mHeader;
    HandlerOpsIntf *mOps;
    SocketServerStateMachine mSSSM;
};

};

#endif

