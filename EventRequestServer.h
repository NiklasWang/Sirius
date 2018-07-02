#ifndef _EVENT_REQUEST_SERVER_H_
#define _EVENT_REQUEST_SERVER_H_

#include "common.h"
#include "SiriusIntf.h"
#include "RunOnceThread.h"
#include "SocketServerStateMachine.h"

namespace sirius {

class HandlerOpsIntf;

class EventRequestServer :
    public RunOnceFunc,
    public noncopyable {
public:
    EventRequestServer(HandlerOpsIntf *ops, int32_t server_socket);
    ~EventRequestServer();
    int32_t construct();
    int32_t destruct();

public:
    int32_t runOnceFunc(void *in, void *out) override;
    int32_t onOnceFuncFinished(int32_t rc) override;
    int32_t abortOnceFunc() override;

private:
    int32_t handleClientMsg(char *msg);
    struct EvtInfo;
    int32_t convertMsgToEvt(char *msg, EvtInfo *evt);

private:
    class RunOnce :
        public RunOnceThread {
    public:
        int32_t run(RunOnceFunc *func, void *in, void *out);
        int32_t exit();
        bool    isRuning();
    };

    struct EvtInfo {
        EvtInfo();
        int32_t evt;
        int32_t arg1;
        int32_t arg2;
    };

private:
    bool        mConstructed;
    ModuleType  mModule;
    RequestType mType;
    int32_t     mSocketFd;
    RunOnce     mRunOnce;
    bool        mExit;
    HandlerOpsIntf          *mOps;
    SocketServerStateMachine mSSSM;
};

};

#endif
