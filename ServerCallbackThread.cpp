#include "ServerCallbackThread.h"

namespace sirius {

int32_t ServerCallbackThread::send(RequestType type, void *buf)
{
    int32_t rc = NO_ERROR;

    Task task;
    task.type = type;
    task.buf = buf;

    rc = newTask(&task);
    if (!SUCCEED(rc)) {
        LOGE(mModule, "Failed to push to thread");
    }

    if (SUCCEED(rc)) {
        task.sync.wait();
        rc = task.rc;
    }

    return rc;
}

int32_t ServerCallbackThread::setCb(callback_func func)
{
    mFunc = func;
    return NO_ERROR;
}

ServerCallbackThread::ServerCallbackThread(callback_func func) :
    mConstructed(false),
    mModule(MODULE_SERVER_CB_THREAD),
    mFunc(func)
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

    if (mFunc != 0) {
        rc = task->rc = mFunc(task->type, task->buf);
    } else {
        LOGE(mModule, "Callback func not set, can't send callback");
    }

    return rc;
}

int32_t ServerCallbackThread::taskDone(void *dat, int32_t rc)
{
    Task *task = static_cast<Task *>(dat);

    if (!SUCCEED(rc)) {
        LOGE(mModule, "Failed to send callback, %d", rc);
    }

    task->sync.signal();

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
    id(mCnt++),
    sync(_sync),
    rc(NO_ERROR),
    type(REQUEST_TYPE_MAX_INVALID),
    buf(NULL)
{
}

};

