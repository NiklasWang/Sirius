#include "ServerCallbackThread.h"
#include "ThreadPoolEx.h"

namespace sirius {

int32_t ServerCallbackThread::send(RequestType type, int32_t id, void *head, void *dat)
{
    int32_t rc = NO_ERROR;

    if (SUCCEED(rc)) {
        rc = mThreads->run(
            [=]() -> int32_t {
                int32_t _rc = NO_ERROR;
                if (NOTNULL(mCbFunc)) {
                    mRequestCnt++;
                    _rc = mCbFunc(type, id, head, dat);
                } else {
                    LOGE(mModule, "Request callback func not set, can't send.");
                    _rc = NOT_INITED;
                }
                return _rc;
            }
        );
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to send request, %d", rc);
        }
    }

    return rc;
}

int32_t ServerCallbackThread::send(int32_t event, int32_t arg1, int32_t arg2)
{
    int32_t rc = NO_ERROR;

    if (SUCCEED(rc)) {
        rc = mThreads->run(
            [=]() -> int32_t {
                int32_t _rc = NO_ERROR;
                if (NOTNULL(mEvtCbFunc)) {
                    mEvtCnt++;
                    _rc = mEvtCbFunc(event, arg1, arg2);
                } else {
                    LOGE(mModule, "Event callback func not set, can't send.");
                    _rc = NOT_INITED;
                }
                return _rc;
            }
        );
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to send event, %d", rc);
        }
    }

    return rc;
}

int32_t ServerCallbackThread::send(int32_t type, void *data, int32_t size)
{
    int32_t rc = NO_ERROR;

    if (SUCCEED(rc)) {
        rc = mThreads->runWait(
            [=]() -> int32_t {
                int32_t _rc = NO_ERROR;
                if (NOTNULL(mDataCbFunc)) {
                    mDataCnt++;
                    _rc = mDataCbFunc(type, data, size);
                } else {
                    LOGE(mModule, "Data callback func not set, can't send.");
                    _rc = NOT_INITED;
                }
                return _rc;
            }
        );
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to send data, %d", rc);
        }
    }

    return rc;
}

int32_t ServerCallbackThread::setCallback(RequestCbFunc requestCb)
{
    mCbFunc = requestCb;
    return NO_ERROR;
}

int32_t ServerCallbackThread::setCallback(EventCbFunc eventCb)
{
    mEvtCbFunc = eventCb;
    return NO_ERROR;
}

int32_t ServerCallbackThread::setCallback(DataCbFunc dataCb)
{
    mDataCbFunc = dataCb;
    return NO_ERROR;
}

ServerCallbackThread::ServerCallbackThread() :
    mConstructed(false),
    mModule(MODULE_SERVER_CB_THREAD),
    mCbFunc(NULL),
    mEvtCbFunc(NULL),
    mDataCbFunc(NULL),
    mThreads(NULL),
    mRequestCnt(0),
    mEvtCnt(0),
    mDataCnt(0)
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

