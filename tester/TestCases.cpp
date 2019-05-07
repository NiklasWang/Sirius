#include "common.h"
#include "ClientTesterImpl.h"

namespace tester {

using namespace sirius;

const ClientTesterImpl::TestCases ClientTesterImpl::gTestCases[] = {
    {
        .type = PREVIEW_NV21,
        .func = &ClientTesterImpl::testPreview,
        .arg  = 0,
    },
    {
        .type = PREVIEW_NV21,
        .func = &ClientTesterImpl::testPreview,
        .arg  = 1,
    },
    {
        .type = PREVIEW_NV21,
        .func = &ClientTesterImpl::testPreview,
        .arg  = 2,
    },
    {
        .type = PREVIEW_NV21,
        .func = &ClientTesterImpl::testPreview,
        .arg  = 3,
    },
    {
        .type = PREVIEW_NV21,
        .func = &ClientTesterImpl::testPreview,
        .arg  = 4,
    },
    {
        .type = PREVIEW_NV21,
        .func = &ClientTesterImpl::testPreview,
        .arg  = 5,
    },
    {
        .type = PREVIEW_NV21,
        .func = &ClientTesterImpl::testPreview,
        .arg  = 6,
    },
    {
        .type = PICTURE_NV21,
        .func = &ClientTesterImpl::testNV21Picture,
        .arg  = 0,
    },
    {
        .type = PICTURE_NV21,
        .func = &ClientTesterImpl::testNV21Picture,
        .arg  = 1,
    },
    {
        .type = PICTURE_NV21,
        .func = &ClientTesterImpl::testNV21Picture,
        .arg  = 2,
    },
    {
        .type = PICTURE_NV21,
        .func = &ClientTesterImpl::testNV21Picture,
        .arg  = 3,
    },
    {
        .type = PICTURE_NV21,
        .func = &ClientTesterImpl::testNV21Picture,
        .arg  = 4,
    },
    {
        .type = PICTURE_NV21,
        .func = &ClientTesterImpl::testNV21Picture,
        .arg  = 5,
    },
    {
        .type = PICTURE_NV21,
        .func = &ClientTesterImpl::testNV21Picture,
        .arg  = 6,
    },
    {
        .type = PICTURE_BAYER,
        .func = &ClientTesterImpl::testBayerPicture,
        .arg  = 0,
    },
    {
        .type = PICTURE_BAYER,
        .func = &ClientTesterImpl::testBayerPicture,
        .arg  = 1,
    },
    {
        .type = PICTURE_BAYER,
        .func = &ClientTesterImpl::testBayerPicture,
        .arg  = 2,
    },
    {
        .type = PICTURE_BAYER,
        .func = &ClientTesterImpl::testBayerPicture,
        .arg  = 3,
    },
    {
        .type = PICTURE_BAYER,
        .func = &ClientTesterImpl::testBayerPicture,
        .arg  = 4,
    },
    {
        .type = PICTURE_BAYER,
        .func = &ClientTesterImpl::testBayerPicture,
        .arg  = 5,
    },
    {
        .type = PICTURE_BAYER,
        .func = &ClientTesterImpl::testBayerPicture,
        .arg  = 6,
    },
    {
        .type = EXTENDED_EVENT,
        .func = &ClientTesterImpl::testEvents,
        .arg  = 0,
    },
    {
        .type = EXTENDED_EVENT,
        .func = &ClientTesterImpl::testEvents,
        .arg  = 1,
    },
    {
        .type = EXTENDED_EVENT,
        .func = &ClientTesterImpl::testEvents,
        .arg  = 2,
    },
    {
        .type = EXTENDED_EVENT,
        .func = &ClientTesterImpl::testEvents,
        .arg  = 3,
    },
    {
        .type = EXTENDED_EVENT,
        .func = &ClientTesterImpl::testEvents,
        .arg  = 4,
    },
    {
        .type = EXTENDED_EVENT,
        .func = &ClientTesterImpl::testEvents,
        .arg  = 5,
    },
    {
        .type = EXTENDED_EVENT,
        .func = &ClientTesterImpl::testEvents,
        .arg  = 6,
    },
    {
        .type = CUSTOM_DATA,
        .func = &ClientTesterImpl::testCustomData,
        .arg  = 0,
    },
    {
        .type = CUSTOM_DATA,
        .func = &ClientTesterImpl::testCustomData,
        .arg  = 1,
    },
    {
        .type = CUSTOM_DATA,
        .func = &ClientTesterImpl::testCustomData,
        .arg  = 2,
    },
    {
        .type = CUSTOM_DATA,
        .func = &ClientTesterImpl::testCustomData,
        .arg  = 3,
    },
    {
        .type = CUSTOM_DATA,
        .func = &ClientTesterImpl::testCustomData,
        .arg  = 4,
    },
    {
        .type = CUSTOM_DATA,
        .func = &ClientTesterImpl::testCustomData,
        .arg  = 5,
    },
    {
        .type = CUSTOM_DATA,
        .func = &ClientTesterImpl::testCustomData,
        .arg  = 6,
    },
    {
        .type = CUSTOM_DATA,
        .func = &ClientTesterImpl::testCustomData,
        .arg  = 7,
    },
    {
        .type = CUSTOM_DATA,
        .func = &ClientTesterImpl::testCustomData,
        .arg  = 8,
    },
    {
        .type = CUSTOM_DATA,
        .func = &ClientTesterImpl::testCustomData,
        .arg  = 9,
    },
    {
        .type = CUSTOM_DATA,
        .func = &ClientTesterImpl::testCustomData,
        .arg  = 10,
    },
    {
        .type = CUSTOM_DATA,
        .func = &ClientTesterImpl::testCustomData,
        .arg  = 11,
    },
    {
        .type = CUSTOM_DATA,
        .func = &ClientTesterImpl::testCustomData,
        .arg  = 12,
    },
    {
        .type = CUSTOM_DATA,
        .func = &ClientTesterImpl::testCustomData,
        .arg  = 13,
    },
    {
        .type = CUSTOM_DATA,
        .func = &ClientTesterImpl::testCustomData,
        .arg  = 14,
    },
    {
        .type = REQUEST_TYPE_MAX_INVALID, // Mixture
        .func = &ClientTesterImpl::testMixture,
        .arg  = 0,
    },
    {
        .type = REQUEST_TYPE_MAX_INVALID, // Mixture
        .func = &ClientTesterImpl::testMixture,
        .arg  = 1,
    },
};

int32_t ClientTesterImpl::getTestRounds(RequestType type)
{
    int32_t result = 0;

    for (uint32_t i = 0; i < ARRAYSIZE(gTestCases); i++) {
        if (type == gTestCases[i].type) {
            result++;
        }
    }

    return result;
}

int32_t ClientTesterImpl::RunTest(RequestType type, int32_t round)
{
    int32_t rc = NOT_FOUND;
    TestFunction func = NULL;
    int32_t arg = 0;
    int32_t index = 0;

    for (uint32_t i = 0; i < ARRAYSIZE(gTestCases); i++) {
        if (type == gTestCases[i].type) {
            if (index == round) {
                func = gTestCases[i].func;
                arg  = gTestCases[i].arg;
                rc = NO_ERROR;
                break;
            }
            index++;
        }
    }

    if (!SUCCEED(rc)) {
        LOGE(mModule, "Not found test function. Request %s round %d, %d",
            getRequestName(type), round, rc);
    }

    if (SUCCEED(rc)) {
        rc = (this->*func)(arg);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Test failed, Request %s round %d, %d",
                getRequestName(type), round, rc);
        }
    }

    return rc;
}

};

