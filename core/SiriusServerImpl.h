#ifndef _SIRIUS_SERVER_IMPL_H_
#define _SIRIUS_SERVER_IMPL_H_

#include "common.h"
#include "SiriusServerIntf.h"
#include "SyncType.h"

namespace sirius {

class SiriusCore;
class ThreadPoolEx;

class SiriusServerImpl :
    public SiriusServerIntf,
    public noncopyable  {
public:
    int32_t request(RequestType type) override;
    int32_t abort(RequestType type) override;
    int32_t enqueue(RequestType type, int32_t id) override;
    int32_t setCallback(RequestCbFunc requestCb);
    int32_t setCallback(EventCbFunc eventCb);
    int32_t setCallback(DataCbFunc dataCb);

private:
    struct BufferInfo {
        RequestType type;
        int32_t id;
    };

    struct CbInfo {
        RequestCbFunc requestCb;
        EventCbFunc   eventCb;
        DataCbFunc    dataCb;
    };

private:
    int32_t coreRequest(void *type);
    int32_t coreAbort(void *type);
    int32_t coreEnqueue(void *info);
    int32_t coreSetCallback(void *func);

public:
    SiriusServerImpl();
    virtual ~SiriusServerImpl();
    int32_t construct();
    int32_t destruct();

private:
    enum TaskType {
        TYPE_REQUEST,
        TYPE_ABORT,
        TYPE_ENQUEUE,
        TYPE_SET_CB,
        TYPE_MAX_INVALID,
    };

public:
    struct TaskBase {
        uint32_t id;
        TaskType type;
        SyncType sync;
        int32_t  final_rc;
        ModuleType module;
        static const char * const kTaskString[];
     public:
        const char *whoamI();
        TaskType getType() { return type; }
        SyncType *getSync() { return &sync; }
        ModuleType getModule() { return module; }
        void setFinalRc(int32_t rc) { final_rc = rc; }
        int32_t getFinalRc() { return final_rc; }
        void setid(uint32_t _id) { id = _id; }
        uint32_t getid() { return id; }
        virtual void *getTask() { return NULL; }
        explicit TaskBase(
            TaskType  _type = TYPE_MAX_INVALID,
            sync_type _sync = SYNC_TYPE) :
            type(_type), sync(_sync), module(MODULE_SIRIUS_IMPL) {}
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

private:
    typedef int32_t (SiriusServerImpl::*PushToThreadFunc)(TaskType type, void *arg);

    template <typename T, sync_type sync = SYNC_TYPE>
    int32_t pushToThread(TaskType type, void *value);

    int32_t processTask(TaskBase *task);
    int32_t taskDone(TaskBase *task, int32_t processRC);

private:
    bool          mConstructed;
    ModuleType    mModule;
    uint32_t      mTaskCnt;
    ThreadPoolEx *mThreads;
    SiriusCore   *mCore;
    static const PushToThreadFunc gAddThreadTaskFunc[];
};

};

#endif
