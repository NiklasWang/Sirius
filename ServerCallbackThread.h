#ifndef _SERVER_CALLBACK_THREAD_H_
#define _SERVER_CALLBACK_THREAD_H_

#include "common.h"
#include "SiriusIntf.h"
#include "QueuedThread.h"
#include "SyncType.h"

namespace sirius {

class ServerCallbackThread :
    public QueuedThread,
    public noncopyable {
public:
    int32_t send(RequestType type, void *buf);
    int32_t setCb(callback_func func);

public:
    ServerCallbackThread(callback_func func = 0);
    virtual ~ServerCallbackThread();

private:
    int32_t processTask(void *dat) override;
    int32_t taskDone(void *dat, int32_t rc) override;

public:
    int32_t construct();
    int32_t destruct();

private:
    struct Task {
        uint32_t id;
        SyncType sync;
        int32_t  rc;
        RequestType type;
        void    *buf;
        static uint32_t mCnt;
     public:
        Task(sync_type _sync = SYNC_TYPE);
        ~Task() = default;
    };

private:
    bool          mConstructed;
    ModuleType    mModule;
    callback_func mFunc;
};

};

#endif
