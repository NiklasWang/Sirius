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
    int32_t send(RequestType type, int32_t id, void *head, void *dat);
    int32_t send(int32_t event, int32_t arg1, int32_t arg2);
    int32_t setCb(RequestCbFunc requestCb, EventCbFunc eventCb);

public:
    ServerCallbackThread();
    virtual ~ServerCallbackThread();

private:
    int32_t processTask(void *dat) override;
    int32_t taskDone(void *dat, int32_t rc) override;

public:
    int32_t construct();
    int32_t destruct();

private:
    struct Task {
    public:
        enum TaskType {
            TASK_TYPE_DAT,
            TASK_TYPE_EVT,
        };

    public:
        TaskType type;
        RequestType request;
        int32_t  tid;
        void    *head;
        void    *dat;
        int32_t  evt;
        int32_t  arg1;
        int32_t  arg2;
        SyncType sync;
        int32_t  rc;
        int32_t  id;
        static uint32_t mCnt;
     public:
        Task(sync_type _sync = SYNC_TYPE);
        ~Task() = default;
    };

private:
    bool          mConstructed;
    ModuleType    mModule;
    RequestCbFunc mCbFunc;
    EventCbFunc   mEvtCbFunc;
};

};

#endif
