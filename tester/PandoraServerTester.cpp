#include "common.h"
#include "SiriusServerIntf.h"
#include "PandoraServerTester.h"

namespace sirius {

int32_t requestCbFunc(RequestType type, int32_t id, void *header, void *dat);
int32_t eventCbFunc(int32_t event, int32_t arg1, int32_t arg2);
int32_t dataCbFunc(int32_t type, void *data, int32_t size);

int32_t ServerTester::prepare()
{
    int32_t rc = NO_ERROR;

    if (SUCCEED(rc)) {
        rc = mSirius.setCallback(requestCbFunc);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to set request callback, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = mSirius.setCallback(eventCbFunc);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to set event callback, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = mSirius.setCallback(dataCbFunc);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to set data callback, %d", rc);
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
        case PICTURE_BAYER: {
            rc = testBayerPicture();
        }; break;
        case EXTENDED_EVENT: {
            rc = testEvents();
        }; break;
        case CUSTOM_DATA: {
            rc = testCustomData();
        }; break;
        default: {
        }; break;
    }

    return rc;
}

int32_t ServerTester::abortUsecase(RequestType request)
{
    int32_t rc = NO_ERROR;

    switch (request) {
        case PREVIEW_NV21: {
            rc = abortPreview();
        }; break;
        case PICTURE_NV21: {
            rc = abortNV21Picture();
        }; break;
        case PICTURE_BAYER: {
            rc = abortBayerPicture();
        }; break;
        case EXTENDED_EVENT: {
            rc = abortEvents();
        }; break;
        case CUSTOM_DATA: {
            rc = abortCustomData();
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

    if (SUCCEED(rc)) {
        rc = mSirius.request(PICTURE_BAYER);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to request bayer picture, %d", rc);
        }
    }

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

    if (SUCCEED(rc)) {
        rc = mSirius.request(CUSTOM_DATA);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to request custom data, %d", rc);
        }
    }

    return rc;
}

int32_t ServerTester::abortPreview()
{
    int32_t rc = NO_ERROR;

    if (SUCCEED(rc)) {
        rc = mSirius.abort(PREVIEW_NV21);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to abort preview, %d", rc);
        }
    }

    return rc;
}

int32_t ServerTester::abortNV21Picture()
{
    int32_t rc = NO_ERROR;

    if (SUCCEED(rc)) {
        rc = mSirius.abort(PICTURE_NV21);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to abort picture, %d", rc);
        }
    }

    return rc;
}

int32_t ServerTester::abortBayerPicture()
{
    int32_t rc = NO_ERROR;

    if (SUCCEED(rc)) {
        rc = mSirius.abort(PICTURE_BAYER);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to abort bayer picture, %d", rc);
        }
    }

    return rc;
}

int32_t ServerTester::abortEvents()
{
    int32_t rc = NO_ERROR;

    if (SUCCEED(rc)) {
        rc = mSirius.abort(EXTENDED_EVENT);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to abort event, %d", rc);
        }
    }

    return rc;
}

int32_t ServerTester::abortCustomData()
{
    int32_t rc = NO_ERROR;

    if (SUCCEED(rc)) {
        rc = mSirius.abort(CUSTOM_DATA);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to abort custom data, %d", rc);
        }
    }

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

int32_t ServerTester::dataCb(int32_t type, void *data, int32_t size)
{
    int32_t rc = NO_ERROR;

    LOGI(mModule, "Received data, 0x%x, %p, %d", type, data, size);

    return rc;
}

ServerTester::ServerTester() :
    mModule(MODULE_SERVER_TESTER)
{
}

ServerTester::~ServerTester()
{
    abortUsecase(PREVIEW_NV21);
    abortUsecase(PICTURE_NV21);
    abortUsecase(PICTURE_BAYER);
    abortUsecase(EXTENDED_EVENT);
    abortUsecase(CUSTOM_DATA);
}


ServerTester gTester;

int32_t requestCbFunc(RequestType type, int32_t id, void *header, void*dat)
{
    return gTester.requestCb(type, id, header, dat);
}

int32_t eventCbFunc(int32_t event, int32_t arg1, int32_t arg2)
{
    return gTester.eventCb(event, arg1, arg2);
}

int32_t dataCbFunc(int32_t type, void *data, int32_t size)
{
    return gTester.dataCb(type, data, size);
}

};

