#ifndef _SIRIUS_CLIENT_CORE_H_
#define _SIRIUS_CLIENT_CORE_H_

#include "common.h"
#include "SiriusIntf.h"
#include "ClientRequestIntf.h"
#include "RunOnceThread.h"
#include "ServerClientControl.h"
#include "IonBufferMgr.h"
#include "SocketClientStateMachine.h"

namespace sirius {

class SiriusClientCore :
    public RunOnceFunc,
    public noncopyable {
public:
    static SiriusClientCore *getInstance();
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
    SiriusClientCore();
    virtual ~SiriusClientCore();
    int32_t construct();
    int32_t destruct();

private:
    int32_t waitForPushRequestMemory();
    int32_t acceptSingleRequestMemory();
    int32_t convertToRequestType(char *msg, RequestType *type);
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
        void *mem;
        int32_t size;
    };

    class ReadyNotifier :
        public SocketClientStateMachine {
    public:
        int32_t init();
        int32_t deinit();
        int32_t notify(RequestType type, int32_t clientfd);
        ReadyNotifier();

    private:
        bool mExit;
    };

private:
    bool         mConstructed;
    ModuleType   mModule;
    ServerClientControl mCtl;
    RunOnce      mRunOnce;
    bool         mServerConnected;
    bool         mCtlReady;
    bool         mAbortOnce;
    MemoryMap   *mMemMap;
    IonBufferMgr mBufMgr;
    SocketClientStateMachine mSC;
    ReadyNotifier            mNotifier;
    pthread_mutex_t          mSocketL;
    static uint32_t          gCnt;
    static pthread_mutex_t   mInsL;
    static SiriusClientCore *gThis;
};

};

#endif
