#include "ServerCallbackThread.h"
#include "ThreadPoolEx.h"

namespace sirius {

int32_t ServerCallbackThread::send(RequestType type, int32_t id, void *head, void *dat)
{
    int32_t rc = NO_ERROR;

    if (SUCCEED(rc)) {
        rc = mThreads->run(
            [this]() -> int32_t {
                if (NOTNULL(mCbFunc)) {
                    mDatCnt++;
                    rc = mCbFunc(type, id, head, dat);
                } else {
                    LOGE(mModule, "Data callback func not set, can't send.");
                    rc = NOT_INITED;
                }
            }
        );
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to run once thread, %d", rc);
        }
    }

    return rc;
}

int32_t ServerCallbackThread::send(int32_t event, int32_t arg1, int32_t arg2)
{
    int32_t rc = NO_ERROR;

    if (SUCCEED(rc)) {
        rc = mThreads->run(
            [this]() -> int32_t {
                if (NOTNULL(mEvtCbFunc)) {
                    mEvtCnt++;
                    rc = mEvtCbFunc(event, arg1, arg2);
                } else {
                    LOGE(mModule, "Data callback func not set, can't send.");
                    rc = NOT_INITED;
                }
            }
        );
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to run once thread, %d", rc);
        }
    }

    return rc;
}

int32_t ServerCallbackThread::setCb(RequestCbFunc requestCb, EventCbFunc eventCb)
{
    mCbFunc    = requestCb;
    mEvtCbFunc = eventCb;
    return NO_ERROR;
}

ServerCallbackThread::ServerCallbackThread() :
    mConstructed(false),
    mModule(MODULE_SERVER_CB_THREAD),
    mCbFunc(NULL),
    mEvtCbFunc(NULL),
    mThreads(NULL),
    mEvtCnt(0),
    mDatCnt(0)
{
}

ServerCallbackThread::~ServerCallbackThread()
{
    if (mConstructed) {
        destruct();
    }
}

int32_t ServerCallbackThread::construct()
{
    int32_t rc = NO_ERROR;

    if (mConstructed) {
        rc = ALREADY_INITED;
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
    }

    return rc;
}

int32_t ServerCallbackThread::destruct()
{
    int32_t rc = NO_ERROR;

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

    if (!SUCCEED(rc)) {
        LOGE(mModule, "Cb thread destructed with error %d", rc);
    } else {
        LOGD(mModule, "Cb thread destructed");
    }

    return rc;
}

};

