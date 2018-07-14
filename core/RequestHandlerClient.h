#ifndef _REQUEST_HANDLER_CLIENT_H_
#define _REQUEST_HANDLER_CLIENT_H_

#include "common.h"
#include "SiriusIntf.h"
#include "ClientRequestIntf.h"
#include "RunOnceThread.h"
#include "ServerClientControl.h"
#include "IonBufferMgr.h"
#include "SocketClientStateMachine.h"

namespace sirius {

class RequestHandlerClient :
    public RunOnceFunc,
    public noncopyable {
public:
    static RequestHandlerClient *getInstance();
    static uint32_t removeInstance();

public:
    int32_t onDataReady(ClientRequestIntf *intf, RequestType type);
    bool requested(RequestType type);

private:
    class RunOnce :
        public RunOnceThread {
    public:
        int32_t run(RunOnceFunc *func, void *in, void *out);
        int32_t exit();
        bool    isRuning();
    };

private:
    RequestHandlerClient();
    virtual ~RequestHandlerClient();
    int32_t construct();
    int32_t destruct();

private:
    int32_t waitForPushRequestMemory();
    int32_t acceptSingleRequestMemory();
    int32_t addMemoryMap(RequestType type,
        void *mem, int32_t fd, int32_t size);
    int32_t findMemoryMap(RequestType type,
        int32_t fd, void **mem, int32_t *size);
    int32_t releaseAllMems();
    int32_t copyToServer(ClientRequestIntf *intf,
        void *dst, int32_t maxSize);

private:
    int32_t runOnceFunc(void *in, void *out) override;
    int32_t onOnceFuncFinished(int32_t rc) override;
    int32_t abortOnceFunc() override;

private:
    struct MemoryMap {
        int32_t fd;
        void   *mem;
        int32_t size;
    };

private:
    bool         mConstructed;
    ModuleType   mModule;
    RunOnce      mRunOnce;
    bool         mAbortOnce;
    MemoryMap   *mMemMap;
    SocketClientStateMachine mSC;

private:
    static SiriusClientCore  mCore;
};

};

#endif
