#ifndef _REQUEST_HANDLER_H_
#define _REQUEST_HANDLER_H_

#include "common.h"
#include "SiriusIntf.h"
#include "HandlerOpsIntf.h"
#include "RequestHandlerIntf.h"
#include "ThreadPoolEx.h"
#include "SocketServerStateMachine.h"

namespace sirius {

class RequestHandler :
    public RequestHandlerIntf {
public:
    virtual int32_t enqueue(int32_t id) override;
    virtual int32_t abort() override;
    virtual int32_t setSocketFd(int32_t fd) override;
    virtual int32_t onClientReady() override;

    virtual RequestType getType() override;
    virtual const char *getName() override;

public:
    virtual int32_t construct() override;
    virtual int32_t destruct() override;
    RequestHandler(HandlerOpsIntf *ops,
        RequestType type, const char *name, uint32_t memNum);
    virtual ~RequestHandler();

protected:
    virtual int32_t getHeaderSize() = 0;
    virtual int32_t getDataSize() = 0;

private:
    virtual int32_t startServerLoop();
    virtual int32_t exitServerLoop();
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

private:
    bool          mConstructed;
    const char   *mName;
    RequestType   mType;
    bool          mMemShared;
    uint32_t      mMemNum;
    MemoryInfo   *mMem;
    ThreadPoolEx *mThreads;

protected:
    ModuleType      mModule;
    Header          mHeader;
    HandlerOpsIntf *mOps;
    SocketServerStateMachine mSSSM;
};

};

#endif

