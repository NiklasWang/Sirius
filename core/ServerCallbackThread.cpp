#include "ServerCallbackThread.h"

namespace sirius {

int32_t ServerCallbackThread::send(RequestType type, int32_t id, void *head, void *dat)
{
    int32_t rc = NO_ERROR;

    Task task;
    task.type = Task::TASK_TYPE_DAT;
    task.request = type;
    task.tid  = id;
    task.head = head;
    task.dat  = dat;

    rc = newTask(&task);
    if (!SUCCEED(rc)) {
        LOGE(mModule, "Failed to push to thread");
    }

    if (SUCCEED(rc)) {
        if (task.sync == SYNC_TYPE) {
            task.sync.wait();
            rc = task.rc;
        }
    }

    return rc;
}

int32_t ServerCallbackThread::send(int32_t event, int32_t arg1, int32_t arg2)
{
    int32_t rc = NO_ERROR;

    Task task;
    task.type = Task::TASK_TYPE_EVT;
    task.evt  = event;
    task.arg1 = arg1;
    task.arg2 = arg2;

    rc = newTask(&task);
    if (!SUCCEED(rc)) {
        LOGE(mModule, "Failed to push to thread");
    }

    if (SUCCEED(rc)) {
        if (task.sync == SYNC_TYPE) {
            task.sync.wait();
            rc = task.rc;
        }
    }

    return rc;
}

int32_t ServerCallbackThread::setCb(RequestCbFunc requestCb, EventCbFunc eventCb)
{
    mCbFunc = requestCb;
    mCbFunc = eventCb;
    return NO_ERROR;
}

ServerCallbackThread::ServerCallbackThread() :
    mConstructed(false),
    mModule(MODULE_SERVER_CB_THREAD),
    mCbFunc(NULL),
    mEvtCbFunc(NULL)
{
}

ServerCallbackThread::~ServerCallbackThread()
{
    if (mConstructed) {
        destruct();
    }
}

int32_t ServerCallbackThread::processTask(void *dat)
{
    int32_t rc = NOT_INITED;
    Task *task = static_cast<Task *>(dat);

    if (task->type == Task::TASK_TYPE_DAT) {
        if (NOTNULL(mCbFunc)) {
            rc = task->rc = mCbFunc(task->request, task->tid, task->head, task->dat);
        } else {
            LOGE(mModule, "Data callback func not set, can't send.");
        }
    }

    if (task->type == Task::TASK_TYPE_EVT) {
        if (NOTNULL(mEvtCbFunc)) {
            rc = task->rc = mEvtCbFunc(task->evt, task->arg1, task->arg2);
        } else {
            LOGE(mModule, "Evt callback func not set, can't send.");
        }
    }

    return rc;
}

int32_t ServerCallbackThread::taskDone(void *dat, int32_t rc)
{
    Task *task = static_cast<Task *>(dat);

    if (!SUCCEED(rc)) {
        LOGE(mModule, "Failed to send callback, %d", rc);
    }

    if (task->sync == SYNC_TYPE) {
        task->sync.signal();
    }

    return NO_ERROR;
}

int32_t ServerCallbackThread::construct()
{
    int32_t rc = NO_ERROR;

    rc = constructThread();
    if (SUCCEED(rc)) {
        mConstructed = true;
    }

    return rc;
}

int32_t ServerCallbackThread::destruct()
{
    int32_t rc = NO_ERROR;

    if (mConstructed) {
        rc = destructThread();
    }

    return rc;
}

uint32_t ServerCallbackThread::Task::mCnt = 0;

ServerCallbackThread::Task::Task(sync_type _sync) :
    type(TASK_TYPE_DAT),
    request(REQUEST_TYPE_MAX_INVALID),
    tid(-1),
    head(NULL),
    dat(NULL),
    evt(-1),
    arg1(-1),
    arg2(-1),
    sync(_sync),
    rc(NO_ERROR),
    id(mCnt++)
{
}

};

