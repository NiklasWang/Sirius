#ifndef _SIRIUS_IMPL_H_
#define _SIRIUS_IMPL_H_

#include "common.h"
#include "SiriusIntf.h"
#include "SyncType.h"
#include "ThreadPool.h"

namespace sirius {

class SiriusCore;

class SiriusImpl :
    public SiriusIntf,
    public ThreadIntf,
    public noncopyable  {
public:
    int32_t request(RequestType type) override;
    int32_t abort(RequestType type) override;
    int32_t enqueue(RequestType type, int32_t id) override;
    int32_t setCallback(RequestCbFunc requestCb, EventCbFunc eventCb) override;

private:
    struct BufferInfo {
        RequestType type;
        int32_t id;
    };

    struct CbInfo {
        RequestCbFunc requestCb;
        EventCbFunc   eventCb;
    };

private:
    int32_t coreRequest(void *type);
    int32_t coreAbort(void *type);
    int32_t coreEnqueue(void *info);
    int32_t coreSetCallback(void *func);

public:
    SiriusImpl();
    virtual ~SiriusImpl();
    int32_t construct();
    int32_t destruct();

private:
    int32_t processTask(void *dat) override;
    int32_t taskDone(void *dat, int32_t processRC) override;

private:
    enum TaskType {
        TYPE_REQUEST,
        TYPE_ABORT,
        TYPE_ENQUEUE,
        TYPE_SET_CB,
        TYPE_MAX_INVALID,
    };

    struct TaskBase {
        uint32_t id;
        TaskType type;
        SyncType sync;
        int32_t final_rc;
        static const char * const kTaskString[];
     public:
        const char *whoamI();
        TaskType getType() { return type; }
        SyncType *getSync() { return &sync; };
        void setFinalRc(int32_t rc) { final_rc = rc; };
        int32_t getFinalRc() { return final_rc; };
        void setid(uint32_t _id) { id = _id; };
        uint32_t getid() { return id; };
        virtual void *getTask() = 0;
        explicit TaskBase(
            TaskType  _type = TYPE_MAX_INVALID,
            sync_type _sync = SYNC_TYPE) :
            type(_type), sync(_sync) {}
        virtual ~TaskBase() {}
    };

    template <class T>
    struct Task :
        public TaskBase {
        T arg;
        explicit Task(
            TaskType  _type = TYPE_MAX_INVALID,
            sync_type _sync = SYNC_TYPE) :
            TaskBase(_type, _sync) {}
        ~Task() = default;
        void *getTask() override { return &arg; };
        void setTask(T *_arg) { arg = *_arg; };
    };

    typedef int32_t (SiriusImpl::*PushToThreadFunc)(TaskType type, void *arg);
    template <typename T, sync_type sync = SYNC_TYPE>
    int32_t pushToThread(TaskType type, void *value);

    struct ThreadPoolEx :
        public ThreadPool,
        public RefBase {
        explicit ThreadPoolEx(ThreadIntf *p, uint32_t c = 0) :
            ThreadPool(p, c) {}
        virtual ~ThreadPoolEx() = default;
    };

private:
    bool             mConstructed;
    ModuleType       mModule;
    RWLock           mIntfLock;
    uint32_t         mTaskCnt;
    sp<ThreadPoolEx> mThreads;
    SiriusCore      *mCore;
    static const PushToThreadFunc gAddThreadTaskFunc[];
};

};

#endif
