#ifndef _SERVER_CALLBACK_THREAD_H_
#define _SERVER_CALLBACK_THREAD_H_

#include "common.h"
#include "SiriusServerIntf.h"

namespace sirius {

class ThreadPoolEx;

class ServerCallbackThread :
    public noncopyable {
public:
    int32_t send(RequestType type, int32_t id, void *head, void *dat);
    int32_t send(int32_t event, int32_t arg1, int32_t arg2);
    int32_t send(int32_t type, void *data, int32_t size);
    int32_t setCallback(RequestCbFunc requestCb);
    int32_t setCallback(EventCbFunc eventCb);
    int32_t setCallback(DataCbFunc dataCb);

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
    DataCbFunc    mDataCbFunc;
    ThreadPoolEx *mThreads;
    uint32_t      mRequestCnt;
    uint32_t      mEvtCnt;
    uint32_t      mDataCnt;
};

};

#endif
