#include "HandlerOpsIntf.h"
#include "EventRequestServer.h"

namespace sirius {

EventRequestServer::EventRequestServer(
    HandlerOpsIntf *ops, int32_t server_socket) :
    mConstructed(false),
    mModule(MODULE_EVT_REQUEST_SERVER),
    mType(EXTENDED_EVENT),
    mSocketFd(server_socket),
    mExit(false),
    mOps(ops)
{
    ASSERT_LOG(mModule, NOTNULL(ops),
        "ops can't be NULL.");
    ASSERT_LOG(mModule, server_socket > 0,
        "server socket not valid, %d", server_socket);
}

EventRequestServer::~EventRequestServer()
{
    if (mConstructed) {
        destruct();
    }
}

int32_t EventRequestServer::construct()
{
    int32_t rc = NO_ERROR;

    if (mConstructed) {
        rc = ALREADY_INITED;
    }

    if (SUCCEED(rc)) {
        rc = mSSSM.construct();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to construct socket sm, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = mSSSM.setServerSocket(mSocketFd);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to set server socket to SM, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = mRunOnce.run(this, NULL, NULL);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to run once thread, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        mConstructed = true;
        LOGD(mModule, "Event request server constructed");
    }

    return rc;
}

int32_t EventRequestServer::destruct()
{
    int32_t rc = NO_ERROR;
    int32_t final = NO_ERROR;

    if (!mConstructed) {
        rc = NOT_INITED;
    } else {
        mConstructed = false;
    }

    if (SUCCEED(rc)) {
        if (mRunOnce.isRuning()) {
            rc = mRunOnce.exit();
            if (!SUCCEED(rc)) {
                final |= rc;
                LOGE(mModule, "Failed to abort run once thread");
                rc = NO_ERROR;
            }
        }
    }

    if (SUCCEED(rc)) {
        rc = mSSSM.destruct();
        if (!SUCCEED(rc)) {
            final |= rc;
            LOGE(mModule, "Failed to destruct socket sm, %d", rc);
            rc = NO_ERROR;
        }
    }

    if (!SUCCEED(final)) {
        LOGE(mModule, "Event request server destructed with error %d", final);
    } else {
        LOGD(mModule, "Event request server destructed");
    }

    return rc;
}

int32_t EventRequestServer::runOnceFunc(void * /*in*/, void * /*out*/)
{
    int32_t rc = NO_ERROR;
    char msg[SOCKET_DATA_MAX_LEN];

    if (SUCCEED(rc)) {
        rc = mSSSM.waitForConnect();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to wait for connection");
        }
    }

    if (SUCCEED(rc)) {
        if (mExit) {
            rc = USER_ABORTED;
        }
    }

    if (SUCCEED(rc)) {
        do {
            rc = mSSSM.receiveMsg(msg, sizeof(msg));
            if (SUCCEED(rc)) {
                rc = handleClientMsg(msg);
                if (!SUCCEED(rc)) {
                    LOGE(mModule, "Failed to handle received msg \"%s\", %d",
                        msg, rc);
                }
            } else if (rc != USER_ABORTED) {
                LOGE(mModule, "Failed to receive msg from client, %d", rc);
            }
        } while (rc != USER_ABORTED && !mExit);
    }

    return rc;
}

EventRequestServer::EvtInfo::EvtInfo() :
    evt(-1),
    arg1(-1),
    arg2(-1)
{
}

int32_t EventRequestServer::handleClientMsg(char *msg)
{
    int32_t rc = NO_ERROR;
    EvtInfo evt;

    if (SUCCEED(rc)) {
        rc = convertMsgToEvt(msg, &evt);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to convert to event, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = mOps->sendCallback(mType, &evt);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to send callback, %d", rc);
        }
    }

    return rc;
}

int32_t EventRequestServer::convertMsgToEvt(
    char *msg, EventRequestServer::EvtInfo *evt)
{
    int32_t rc = NO_ERROR;
    int32_t cnt = 0;

    cnt = sscanf(msg, SOCKET_EVENT_REQUEST_FORMAT " %d %d %d",
        &evt->evt, &evt->arg1, &evt->arg2);

    if (cnt != 3 || evt->evt < 0 ||
        evt->arg1 < 0 || evt->arg2 < 0) {
        rc = PARAM_INVALID;
    }

    return rc;
}


int32_t EventRequestServer::onOnceFuncFinished(int32_t /*rc*/)
{
    return NO_ERROR;
}

int32_t EventRequestServer::abortOnceFunc()
{
    int32_t rc = NO_ERROR;

    mExit = true;

    if (!mSSSM.connected()) {
        rc = mSSSM.cancelWaitConnect();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to cancel wait client");
        }
    }

    if (mSSSM.waitingMsg()) {
        rc = mSSSM.cancelWaitMsg();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to cancel wait message");
        }
    }

    return rc;
}

int32_t EventRequestServer::RunOnce::run(
    RunOnceFunc *func, void *in, void *out)
{
    int32_t rc = NO_ERROR;

    if (ISNULL(func)) {
        LOGE(MODULE_RUN_ONCE_THREAD, "func can't be null");
        rc = PARAM_INVALID;
    }

    if (SUCCEED(rc)) {
        rc = RunOnceThread::run(func, in, out);
        if (!SUCCEED(rc)) {
            LOGE(MODULE_RUN_ONCE_THREAD, "Failed to start thread");
        }
    }

    return rc;
}

int32_t EventRequestServer::RunOnce::exit()
{
    return RunOnceThread::exit();
}

bool EventRequestServer::RunOnce::isRuning()
{
    return RunOnceThread::isRuning();
}

};

