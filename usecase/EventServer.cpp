#include "EventServer.h"

#define EXTENDED_EVENT_MEMORY_NUM 0

namespace sirius {

int32_t EventServer::getHeaderSize()
{
    return 0;
}

int32_t EventServer::getDataSize()
{
    return 0;
}

int32_t EventServer::startServerLoop()
{
    int32_t rc = NO_ERROR;

    if (SUCCEED(rc)) {
        rc = mSSSM.sendMsg(SOCKET_SERVER_SHARE_DONE,
            strlen(SOCKET_SERVER_SHARE_DONE));
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to send share done msg, %d", rc);
        }
    }

    do {
        EvtInfo info;
        RESETRESULT(rc);
        char msg[SOCKET_DATA_MAX_LEN];
        if (SUCCEED(rc)) {
            msg[0] = '\0';
            rc = mSSSM.receiveMsg(msg, SOCKET_DATA_MAX_LEN);
            msg[SOCKET_DATA_MAX_LEN - 1] = '\0';
            if (rc == USER_ABORTED) {
                LOGI(mModule, "Abort to read client msg.");
            } else if (!SUCCEED(rc)) {
                LOGE(mModule, "Failed to receive clent msg, %d", msg);
            }
        }

        if (SUCCEED(rc)) {
            rc = convertMsgToEvt(msg, &info);
            if (!SUCCEED(rc)) {
                LOGE(mModule, "Failed to convert, %s", msg);
            }
        }

        if (SUCCEED(rc)) {
            rc = onNewEvent(info);
            if (!SUCCEED(rc)) {
                LOGE(mModule, "Failed to send evt, %d", rc);
            }
        }
    } while (rc != USER_ABORTED);

    return rc;
}

EventServer::EventServer(HandlerOpsIntf *ops) :
    RequestHandler(ops,
        EXTENDED_EVENT,
        "Extended Events Request",
        EXTENDED_EVENT_MEMORY_NUM)
{}

EventServer::~EventServer()
{}

EventServer::EvtInfo::EvtInfo() :
    evt(-1),
    arg1(-1),
    arg2(-1)
{}

int32_t EventServer::convertMsgToEvt(char *msg, EvtInfo &info)
{
    int32_t rc = NO_ERROR;

    if (SUCCEED(rc)) {
        int32_t len = strlen(SOCKET_EVENT_REQUEST_FORMAT);
        if (!COMPARE_SAME_STRING(msg,
            SOCKET_EVENT_REQUEST_FORMAT, len)) {
            LOGE(mModule, "Invalid evt msg, %s", msg);
            rc = PARAM_INVALID;
        }
    }

    if (SUCCEED(rc)) {
        EvtInfo _info;
        int32_t len = sscanf(msg, SOCKET_EVENT_REQUEST_FORMAT "%d %d %d",
            &_info.evt, &_info.arg1, &_info.arg2);
        if (len != 3) {
            LOGE(mModule, "Failed to convert evt info from msg, %s", msg);
            rc = INVALID_FORMAT;
        } else {
            info = _info;
        }
    }

    return rc;
}

int32_t EventServer::onNewEvent(EvtInfo &info)
{
    return mOps->send(info.evt, info.arg1, info.arg2);
}

};

