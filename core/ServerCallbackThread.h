#ifndef _SERVER_CALLBACK_THREAD_H_
#define _SERVER_CALLBACK_THREAD_H_

#include "common.h"
#include "SiriusIntf.h"

namespace sirius {

class ThreadPoolEx;

class ServerCallbackThread :
    public noncopyable {
public:
    int32_t send(RequestType type, int32_t id, void *head, void *dat);
    int32_t send(int32_t event, int32_t arg1, int32_t arg2);
    int32_t setCb(RequestCbFunc requestCb, EventCbFunc eventCb);

public:
    ServerCallbackThread();
    virtual ~ServerCallbackThread();

public:
    int32_t construct();
    int32_t destruct();

private:
    bool          mConstructed;
    ModuleType    mModule;
    RequestCbFunc mCbFunc;
    EventCbFunc   mEvtCbFunc;
    ThreadPoolEx *mThreads;
    uint32_t      mEvtCnt;
    uint32_t      mDatCnt;
};

};

#endif
