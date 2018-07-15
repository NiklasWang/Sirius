#ifndef _SIRIUS_CLIENT_CORE_H_
#define _SIRIUS_CLIENT_CORE_H_

#include "common.h"
#include "SiriusIntf.h"
#include "ServerClientControl.h"
#include "BufferMgrIntf.h"
#include "SocketClientStateMachine.h"

namespace sirius {

class SiriusClientCore :
    public noncopyable {
public:
    int32_t prepare();
    bool ready();

    bool requested(RequestType type);
    int32_t importBuf(void **buf, int32_t fd, int32_t len);
    int32_t releaseBuf(void *buf);
    int32_t getUsedMem(RequestType type, int32_t *fd);
    int32_t setMemStatus(RequestType type, int32_t fd, bool fresh = USED_MEMORY);
    int32_t getMemStatus(RequestType type, int32_t fd, bool *fresh);
    int32_t getMemSize(RequestType type, int32_t *size);
    int32_t setHeader(Header &header);

public:
    int32_t construct();
    int32_t destruct();
    SiriusClientCore();
    virtual ~SiriusClientCore();

private:
    bool           mConstructed;
    ModuleType     mModule;
    bool           mConnected;
    bool           mReady;
    BufferMgrIntf *mBufMgr;
    void          *mCtlBuf;
    ServerClientControl      mCtl;
    pthread_mutex_t          mLocker;
    SocketClientStateMachine mSC;
};

};

#endif
