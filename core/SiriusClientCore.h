#ifndef _SIRIUS_CLIENT_CORE_H_
#define _SIRIUS_CLIENT_CORE_H_

#include "common.h"
#include "SiriusIntf.h"
#include "RunOnceThread.h"
#include "ServerClientControl.h"
#include "IonBufferMgr.h"
#include "SocketClientStateMachine.h"

namespace sirius {

class SiriusClientCore :
    public HandlerOpsClientIntf,
    public RunOnceFunc,
    public noncopyable {
public:
    bool requested(RequestType type);
    bool connected();
    bool ready();
    int32_t importIon(void **buf, int32_t len, int32_t *fd) override;
    int32_t releaseIon(void *buf) override;
    int32_t setMemStatus(RequestType type, int32_t fd, bool fresh = USED_MEMORY) override;
    int32_t getMemStatus(RequestType type, int32_t fd, bool *fresh) override;
    int32_t getMemSize(RequestType type, int32_t *size) override;

public:
    int32_t construct();
    int32_t destruct();
    SiriusClientCore();
    virtual ~SiriusClientCore();

private:
    int32_t runOnceFunc(void *in, void *out) override;
    int32_t onOnceFuncFinished(int32_t rc) override;
    int32_t abortOnceFunc() override;

private:
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
    bool         mConnected;
    bool         mReady;
    bool         mAbortOnce;
    RunOnce      mRunOnce;
    IonBufferMgr mBufMgr;
    ServerClientControl mCtl;
    SocketClientStateMachine mSC;
};

};

#endif
