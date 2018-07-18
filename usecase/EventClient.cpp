#include "common.h"
#include "EventClient.h"

namespace sirius {

EventClient::EventClient() :
    RequestHandlerClient(
        EXTENDED_EVENT,
        "Extended Event Request Client",
        REQUEST_HANDLER_NO_MEMORY_SHARE),
    mModule(MODULE_PREVIEW_REQUEST)
{
}

EventClient::~EventClient()
{
    destruct();
}

int32_t EventClient::construct()
{
    return RequestHandlerClient::construct();
}

int32_t EventClient::destruct()
{
    return RequestHandlerClient::destruct();
}

int32_t EventClient::onEventReady(int32_t event, int32_t arg1, int32_t arg2)
{
    int32_t rc = NO_ERROR;

    if (SUCCEED(rc)) {
        if (!requested()) {
            LOGD(mModule, "%d not requested, abandon this time.", getType());
            rc = NOT_REQUIRED;
        }
    }

    if (SUCCEED(rc)) {
        if (!mReady) {
            rc = prepare();
            if (!SUCCEED(rc)) {
                LOGE(mModule, "Failed to prepare request client handler, %d", rc);
            }
        }
    }

    if (SUCCEED(rc)) {
        char msg[SOCKET_DATA_MAX_LEN];
        sprintf(msg, SOCKET_EVENT_REQUEST_FORMAT "%d %d %d", event, arg1, arg2);
        rc = mSC.sendMsg(msg, strlen(msg));
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to send msg to server, %d", rc);
        }
    }

    return rc;
}

int32_t EventClient::sizeOfHeader()
{
    return 0;
}

int32_t EventClient::sizeOfData(void * /*header*/)
{
    return 0;
}

int32_t EventClient::copyDataToServer(
    uint8_t * /*dst*/, void * /*header*/, uint8_t * /*src*/)
{
    return NO_ERROR;
}

};

