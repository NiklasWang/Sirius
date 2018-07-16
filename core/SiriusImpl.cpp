#include "SiriusImpl.h"
#include "SiriusCore.h"
#include "ThreadPoolEx.h"

namespace sirius {

SiriusImpl::SiriusImpl() :
    mConstructed(false),
    mModule(MODULE_SIRIUS_IMPL),
    mTaskCnt(0),
    mCore(NULL)
{
    LOGI(mModule, "%s %s initializing...", PROJNAME, VERSION);
}

SiriusImpl::~SiriusImpl()
{
    if (mConstructed) {
        destruct();
    }
}

int32_t SiriusImpl::construct()
{
    int32_t rc = NO_ERROR;
    RWLock::AutoWLock l(mIntfLock);

    if (mConstructed) {
        rc = ALREADY_INITED;
    }

    if (SUCCEED(rc)) {
        mCore = new SiriusCore();
        if (ISNULL(mCore)) {
            LOGE(mModule, "Failed to create core");
            rc = UNKNOWN_ERROR;
        }
    }

    if (SUCCEED(rc)) {
        rc = mCore->construct();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to construct core");
        }
    }

    if (SUCCEED(rc)) {
        mThreads = ThreadPoolEx::getInstance();
        if (ISNULL(mThreads)) {
            LOGE(mModule, "Failed to get thread pool");
            rc = NOT_READY;
        }
    }

    if (SUCCEED(rc)) {
        mConstructed = true;
        LOGD(mModule, "Sirius impl constructed");
    }

    return RETURNIGNORE(rc, ALREADY_EXISTS);
}

int32_t SiriusImpl::destruct()
{
    int32_t rc = NO_ERROR;
    RWLock::AutoWLock l(mIntfLock);

    if (!mConstructed) {
        rc = NOT_INITED;
    } else {
        mConstructed = false;
    }

    if (SUCCEED(rc)) {
        if (NOTNULL(mThreads)) {
            mThreads->removeInstance();
            mThreads = NULL;
        }
    }

    if (SUCCEED(rc)) {
        rc = mCore->destruct();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to destruct core");
        } else {
            SECURE_DELETE(mCore);
        }
    }

    if (!SUCCEED(rc)) {
        mConstructed = true;
        LOGE(mModule, "Failed to destruct Sirius impl");
    } else {
        LOGD(mModule, "Sirius impl destructed");
    }

    return RETURNIGNORE(rc, NOT_INITED);

}

const char * const SiriusImpl::TaskBase::kTaskString[] = {
    [SiriusImpl::TYPE_REQUEST]     = "request",
    [SiriusImpl::TYPE_ABORT]       = "abort",
    [SiriusImpl::TYPE_ENQUEUE]     = "enqueue buf",
    [SiriusImpl::TYPE_SET_CB]      = "set callback",
    [SiriusImpl::TYPE_MAX_INVALID] = "max invalid",
};

const char *SiriusImpl::TaskBase::whoamI()
{
    return kTaskString[
        (type < 0 || type > TYPE_MAX_INVALID) ?
        TYPE_MAX_INVALID : type];
}

int32_t SiriusImpl::processTask(TaskBase *task)
{
    assert(!ISNULL(dat));
    uint32_t rc = NO_ERROR;
    TaskType type = NOTNULL(task) ? task->getType() : TYPE_MAX_INVALID;
    void *arg = NOTNULL(task) ? task->getTask() : NULL;

    switch (type) {
        case TYPE_REQUEST: {
            rc = coreRequest(arg);
        } break;
        case TYPE_ABORT: {
            rc = coreAbort(arg);
        } break;
        case TYPE_ENQUEUE: {
            rc = coreEnqueue(arg);
        } break;
        case TYPE_SET_CB: {
            rc = coreSetCallback(arg);
        } break;
        case TYPE_MAX_INVALID:
        default: {
            LOGE(mModule, "Invalid task type %s", task->whoamI());
            rc = UNKNOWN_ERROR;
        } break;
    }

    task->setFinalRc(rc);

    return rc;
}

int32_t SiriusImpl::taskDone(TaskBase *task, int32_t processRC)
{
    int32_t rc = NO_ERROR;
    SyncType *sync = NOTNULL(task) ? task->getSync() : NULL;

    if (!SUCCEED(processRC)) {
        LOGE(mModule, "Failed to process task %s with %d, trying to ignore",
            task->whoamI(), processRC);
    }

    if (NOTNULL(sync) && *sync == SYNC_TYPE) {
        sync->signal();
    } else {
        SECURE_DELETE(task);
    }

    return rc;
}

template <typename T, sync_type sync>
int32_t SiriusImpl::pushToThread(TaskType type, void *value)
{
    int32_t rc = NO_ERROR;
    Task<T> *task = NULL;
    T *arg = static_cast<T *>(value);

    task = new Task<T>(type, sync);
    if (ISNULL(task)) {
        LOGE(mModule, "Failed to create new task");
        rc = NO_MEMORY;
    } else {
        task->setid(mTaskCnt++);
        task->setFinalRc(NO_ERROR);
        if (NOTNULL(arg)) {
            task->setTask(arg);
        }
    }

    if (SUCCEED(rc)) {
        rc = mThreads->run(
            [this](TaskBase *_task) -> int32_t {
                int32_t _rc = processTask(_task);
                taskDone(_task, _rc);
                return _rc;
            },
            task,
            sync
        );
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to run task %d %s",
                task->getid(), task->whoamI());
        }
    }

    if (SUCCEED(rc)) {
        SyncType *syncc = task->getSync();
        if (NOTNULL(syncc) && *syncc == SYNC_TYPE) {
            syncc->wait();
            rc = task->getFinalRc();
            SECURE_DELETE(task);
        }
    }

    return rc;
}

const SiriusImpl::PushToThreadFunc
    SiriusImpl::gAddThreadTaskFunc[] = {
    [SiriusImpl::TYPE_REQUEST] = &SiriusImpl::pushToThread<RequestType, SYNC_TYPE>,
    [SiriusImpl::TYPE_ABORT]   = &SiriusImpl::pushToThread<RequestType, SYNC_TYPE>,
    [SiriusImpl::TYPE_ENQUEUE] = &SiriusImpl::pushToThread<BufferInfo,  SYNC_TYPE>,
    [SiriusImpl::TYPE_SET_CB]  = &SiriusImpl::pushToThread<CbInfo,      SYNC_TYPE>,
};

#define CONSTRUCT_IMPL() \
    do { \
        int32_t rc = NO_ERROR; \
        if (!mConstructed) { \
            rc = construct(); \
            if (!SUCCEED(rc)) { \
                LOGE(mModule, "Failed to construct Sirius impl %d", rc); \
                return rc; \
            } \
        } \
    } while(0)

int32_t SiriusImpl::request(RequestType type)
{
    CONSTRUCT_IMPL();
    RWLock::AutoRLock l(mIntfLock);
    return (this->*(gAddThreadTaskFunc[TYPE_REQUEST]))(TYPE_REQUEST, &type);
}

int32_t SiriusImpl::abort(RequestType type)
{
    CONSTRUCT_IMPL();
    RWLock::AutoRLock l(mIntfLock);
    return (this->*(gAddThreadTaskFunc[TYPE_ABORT]))(TYPE_ABORT, &type);
}

int32_t SiriusImpl::enqueue(RequestType type, int32_t id)
{
    CONSTRUCT_IMPL();
    RWLock::AutoRLock l(mIntfLock);
    BufferInfo buf = {
        .type = type,
        .id   = id,
    };
    return (this->*(gAddThreadTaskFunc[TYPE_ENQUEUE]))(TYPE_ENQUEUE, &buf);
}

int32_t SiriusImpl::setCallback(RequestCbFunc requestCb, EventCbFunc eventCb)
{
    CONSTRUCT_IMPL();
    RWLock::AutoRLock l(mIntfLock);
    CbInfo func = {
        .requestCb = requestCb,
        .eventCb   = eventCb,
    };
    return (this->*(gAddThreadTaskFunc[TYPE_SET_CB]))(TYPE_SET_CB, &func);
}

int32_t SiriusImpl::coreRequest(void *_type)
{
    RequestType *type = static_cast<RequestType *>(_type);
    return ISNULL(mCore) ?
        NOT_INITED : mCore->request(*type);
}

int32_t SiriusImpl::coreAbort(void *_type)
{
    RequestType *type = static_cast<RequestType *>(_type);
    return ISNULL(mCore) ?
        NOT_INITED : mCore->abort(*type);
}

int32_t SiriusImpl::coreEnqueue(void *_info)
{
    BufferInfo *inf = static_cast<BufferInfo *>(_info);
    return ISNULL(mCore) ?
        NOT_INITED : mCore->enqueue(inf->type, inf->id);
}

int32_t SiriusImpl::coreSetCallback(void *_func)
{
    CbInfo *func = static_cast<CbInfo *>(_func);
    return ISNULL(mCore) ?
        NOT_INITED : mCore->setCallback(func->requestCb, func->eventCb);
}

};

