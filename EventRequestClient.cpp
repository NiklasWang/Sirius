#include "common.h"
#include "EventRequestClient.h"
#include "SiriusClientCore.h"

namespace sirius {

int32_t EventRequestClient::sendEvent(
    int32_t evt, int32_t arg1, int32_t arg2)
{
    int32_t rc = NO_ERROR;

    if (SUCCEED(rc)) {
        if (ISNULL(mCore)) {
            LOGE(mModule, "Not constructed.");
            rc = NOT_INITED;
        }
    }

    if (SUCCEED(rc)) {
        if (!mCore->requested(mType)) {
            LOGE(mModule, "Event not requested, abandon evt "
                "%d %d %dthis time.", evt, arg1, arg2);
            rc = NOT_REQUIRED;
        }
    }

    if (SUCCEED(rc)) {
        if (!mSCSM.connected()) {
            rc = mSCSM.connectServer();
            if (!SUCCEED(rc)) {
                LOGE(mModule, "Failed to connect to server, "
                    "abandon evt %d %d %d this time.", evt, arg1, arg2);
            }
        }
    }

    if (SUCCEED(rc)) {
        EvtInfo e(evt, arg1, arg2);
        rc = convertEvtToMsg(mMsg, &e);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to convert evt to msg, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = mSCSM.sendMsg(mMsg, strlen(mMsg));
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to send msg \"%s\" to server, %d", mMsg, rc);
        }
    }

    return rc;
}

int32_t EventRequestClient::convertEvtToMsg(char *msg, EvtInfo *evt)
{
    sprintf(msg, SOCKET_EVENT_REQUEST_FORMAT " %d %d %d",
        evt->evt, evt->arg1, evt->arg2);

    return NO_ERROR;
}

EventRequestClient::EvtInfo::EvtInfo(
    int32_t _evt, int32_t _arg1, int32_t _arg2) :
    evt(_evt),
    arg1(_arg1),
    arg2(_arg2)
{
}

EventRequestClient::EventRequestClient() :
    mConstructed(false),
    mModule(MODULE_EVT_REQUEST_CLIENT),
    mType(EXTENDED_EVENT),
    mName("Extend Event Request"),
    mCore(NULL)
{}

EventRequestClient::~EventRequestClient()
{
    if (mConstructed) {
        destruct();
    }
}

int32_t EventRequestClient::construct()
{
    int32_t rc = NO_ERROR;

    if (ISNULL(mCore)) {
        mCore = SiriusClientCore::getInstance();
        if (ISNULL(mCore)) {
            LOGE(mModule, "Failed to construct event request");
            rc = UNKNOWN_ERROR;
        }
    }

    if (SUCCEED(rc)) {
        rc = mSCSM.construct();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to construct socket state machine, %d", rc);
        }
    }

    return rc;
}

int32_t EventRequestClient::destruct()
{
    if (NOTNULL(mCore)) {
        mCore = NULL;
        SiriusClientCore::removeInstance();
    }

    if (!mSCSM.construct()) {
        LOGE(mModule, "Failed to destruct socket state machine.");
    }

    return NO_ERROR;
}

RequestType EventRequestClient::type()
{
    return mType;
}

const char *EventRequestClient::name()
{
    return mName;
}

};

