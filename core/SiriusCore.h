#ifndef _SIRIUS_CORE_H_
#define _SIRIUS_CORE_H_

#include "common.h"
#include "SiriusIntf.h"
#include "RunOnceThread.h"
#include "HandlerOpsIntf.h"
#include "UserBufferMgr.h"
#include "IonBufferMgr.h"
#include "ServerClientControl.h"
#include "SocketServerStateMachine.h"
#include "ServerCallbackThread.h"
#include "RequestHandler.h"

namespace sirius {

class EventServer;

class SiriusCore :
    public SiriusIntf,
    public HandlerOpsIntf,
    public RunOnceFunc,
    public noncopyable {
public:
    int32_t request(RequestType type) override;
    int32_t abort(RequestType type) override;
    int32_t enqueue(RequestType type, int32_t id) override;
    int32_t setCallback(RequestCbFunc requestCb, EventCbFunc eventCb) override;

public:
    int32_t send(RequestType type, int32_t id, void *head, void *dat) override;
    int32_t send(int32_t event, int32_t arg1, int32_t arg2) override;
    int32_t allocateBuf(void **buf, int32_t len, int32_t *fd) override;
    int32_t releaseBuf(void *buf) override;
    int32_t setMemStatus(RequestType type, int32_t fd, bool fresh = false) override;
    int32_t getMemStatus(RequestType type, int32_t fd, bool *fresh) override;
    int32_t setMemSize(RequestType type, int32_t size) override;
    int32_t getMemSize(RequestType type, int32_t *size) override;
    int32_t addMemory(RequestType type, int32_t clientfd, bool fresh = false) override;
    int32_t setRequestedMark(RequestType type, bool enable = false) override;
    int32_t getHeader(Header &header) override;

public:
    int32_t runOnceFunc(void *in, void *out) override;
    int32_t onOnceFuncFinished(int32_t rc) override;
    int32_t abortOnceFunc() override;

public:
    SiriusCore();
    virtual ~SiriusCore();
    int32_t construct();
    int32_t destruct();

private:
    bool requested(RequestType type);
    int32_t createRequestHandler(RequestType type);
    RequestHandler *createHandler(RequestType type);
    int32_t enableCachedRequests();
    int32_t convertToRequestType(char *msg, RequestType *type);

private:
    class RunOnce :
        public RunOnceThread {
    public:
        int32_t run(RunOnceFunc *func, void *in, void *out);
        int32_t exit();
        bool    isRuning();
    };

private:
    bool                     mConstructed;
    ModuleType               mModule;
    bool                     mExit;
    bool                     mClientReady;
    int32_t                  mCtlFd;
    void                    *mCtlMem;
    ServerClientControl      mCtl;
    SocketServerStateMachine mSS;
    char                     mSocketMsg[SOCKET_DATA_MAX_LEN];
    BufferMgr                mBuffer;
    ServerCallbackThread     mCb;
    RunOnce                 *mRunOnce;
    RequestHandlerIntf      *mRequests[REQUEST_TYPE_MAX_INVALID];
    bool                     mCachedRequest[REQUEST_TYPE_MAX_INVALID];

private:
    static const RequestType gRequestTypeMap[REQUEST_TYPE_MAX_INVALID];
};

};

#endif
