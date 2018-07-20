#include "common.h"
#include "SiriusIntf.h"
#include "ServerTester.h"

namespace sirius {

int32_t requestCbFunc(RequestType type, int32_t id, void *header, void*dat);
int32_t eventCbFunc(int32_t event, int32_t arg1, int32_t arg2);

int32_t ServerTester::prepare()
{
    int32_t rc = NO_ERROR;

    if (SUCCEED(rc)) {
        rc = mSirius.setCallback(requestCbFunc, eventCbFunc);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to set callback, %d", rc);
        }
    }

    return rc;
}

int32_t ServerTester::testUsecase(RequestType request)
{
    int32_t rc = NO_ERROR;

    switch (request) {
        case PREVIEW_NV21: {
            rc = testPreview();
        }; break;
        case PICTURE_NV21: {
            rc = testNV21Picture();
        }; break;
        case EXTENDED_EVENT: {
            rc = testEvents();
        }; break;
        default: {
        }; break;
    }

    return rc;
}

int32_t ServerTester::testPreview()
{
    int32_t rc = NO_ERROR;

    if (SUCCEED(rc)) {
        rc = mSirius.request(PREVIEW_NV21);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to request preview, %d", rc);
        }
    }

    return rc;
}

int32_t ServerTester::testNV21Picture()
{
    int32_t rc = NO_ERROR;

    if (SUCCEED(rc)) {
        rc = mSirius.request(PICTURE_NV21);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to request picture, %d", rc);
        }
    }

    return rc;
}

int32_t ServerTester::testBayerPicture()
{
    int32_t rc = NO_ERROR;

    return rc;
}

int32_t ServerTester::testEvents()
{
    int32_t rc = NO_ERROR;

    if (SUCCEED(rc)) {
        rc = mSirius.request(EXTENDED_EVENT);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to request event, %d", rc);
        }
    }

    return rc;
}

int32_t ServerTester::testCustomData()
{
    int32_t rc = NO_ERROR;

    return rc;
}

int32_t ServerTester::requestCb(
    RequestType type, int32_t id, void *header, void *dat)
{
    int32_t rc = NO_ERROR;

    LOGI(mModule, "Received data cb, %d %d %p %p", type, id, header, dat);

    if (SUCCEED(rc)) {
        rc = mSirius.enqueue(type, id);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to return buffer back, %d", rc);
        }
    }

    return rc;
}

int32_t ServerTester::eventCb(
    int32_t event, int32_t arg1, int32_t arg2)
{
    int32_t rc = NO_ERROR;

    LOGI(mModule, "Received event, 0x%x, %d, %d", event, arg1, arg2);

    return rc;
}

ServerTester::ServerTester() :
    mModule(MODULE_TESTER)
{
}

ServerTester::~ServerTester()
{
}


extern ServerTester gTester;

int32_t requestCbFunc(RequestType type, int32_t id, void *header, void*dat)
{
    return gTester.requestCb(type, id, header, dat);
}

int32_t eventCbFunc(int32_t event, int32_t arg1, int32_t arg2)
{
    return gTester.eventCb(event, arg1, arg2);
}

};

