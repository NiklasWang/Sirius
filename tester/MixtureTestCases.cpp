#include "common.h"
#include "ClientTesterImpl.h"
#include "ServerTesterImpl.h"
#include "YuvPictureOperator.h"

#define ROUND3_TEST_COUNT 6

namespace tester {

using namespace sirius;

const ClientTesterImpl::TestCaseFunction ClientTesterImpl::gPreviewSingleTestCases[] = {
    [0] = &ClientTesterImpl::testPreviewRound1,
    [1] = &ClientTesterImpl::testPreviewRound2,
    [2] = &ClientTesterImpl::testPreviewRound3,
    [3] = &ClientTesterImpl::testPreviewRound4,
};

const ClientTesterImpl::TestCaseFunction ClientTesterImpl::gNV21PictureSingleTestCases[] = {
    [0] = &ClientTesterImpl::testNV21PictureRound1,
    [1] = &ClientTesterImpl::testNV21PictureRound2,
    [2] = &ClientTesterImpl::testNV21PictureRound3,
    [3] = &ClientTesterImpl::testNV21PictureRound5,
};

const ClientTesterImpl::TestCaseFunction ClientTesterImpl::gBayerPictureSingleTestCases[] = {
    [0] = &ClientTesterImpl::testBayerPictureRound1,
    [1] = &ClientTesterImpl::testBayerPictureRound2,
    [2] = &ClientTesterImpl::testBayerPictureRound3,
    [3] = &ClientTesterImpl::testBayerPictureRound5,
};

const ClientTesterImpl::TestCaseFunction ClientTesterImpl::gEventSingleTestCases[] = {
    [0] = &ClientTesterImpl::testEventsRound1,
    [1] = &ClientTesterImpl::testEventsRound2,
    [2] = &ClientTesterImpl::testEventsRound3,
    [3] = &ClientTesterImpl::testEventsRound4,
};

int32_t ClientTesterImpl::testMixtureRound1(int32_t *count)
{
    int32_t rc = NO_ERROR;
    int32_t final = rc;
    Semaphore sem;

    if (SUCCEED(rc)) {
        rc = mThreads->run(
            [&]() -> int32_t {
                int32_t _rc = NO_ERROR;
                for (uint32_t i = 0; i < ARRAYSIZE(gPreviewSingleTestCases); i++) {
                    int32_t tmpCount = 0;
                    _rc = (this->*(gPreviewSingleTestCases[i]))(&tmpCount);
                    if (!SUCCEED(_rc)) {
                        LOGE(mModule, "Failed to run preview single test cases,"
                            " round %d, %d", i, _rc);
                    }
                    *count += tmpCount;
                }
                sem.signal();
                return _rc;
            }
        );
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to run all single test cases async, %d", rc);
            final |= rc;
            RESETRESULT(rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = mThreads->run(
            [&]() -> int32_t {
                int32_t _rc = NO_ERROR;
                for (uint32_t i = 0; i < ARRAYSIZE(gNV21PictureSingleTestCases); i++) {
                    int32_t tmpCount = 0;
                    _rc = (this->*(gNV21PictureSingleTestCases[i]))(&tmpCount);
                    if (!SUCCEED(_rc)) {
                        LOGE(mModule, "Failed to run nv21 picture single test cases,"
                            " round %d, %d", i, _rc);
                    }
                    *count += tmpCount;
                }
                sem.signal();
                return _rc;
            }
        );
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to run all single test cases async, %d", rc);
            final |= rc;
            RESETRESULT(rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = mThreads->run(
            [&]() -> int32_t {
                int32_t _rc = NO_ERROR;
                for (uint32_t i = 0; i < ARRAYSIZE(gBayerPictureSingleTestCases); i++) {
                    int32_t tmpCount = 0;
                    _rc = (this->*(gBayerPictureSingleTestCases[i]))(&tmpCount);
                    if (!SUCCEED(_rc)) {
                        LOGE(mModule, "Failed to run bayer picture single test cases,"
                            " round %d, %d", i, _rc);
                    }
                    *count += tmpCount;
                }
                sem.signal();
                return _rc;
            }
        );
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to run all single test cases async, %d", rc);
            final |= rc;
            RESETRESULT(rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = mThreads->run(
            [&]() -> int32_t {
                int32_t _rc = NO_ERROR;
                for (uint32_t i = 0; i < ARRAYSIZE(gEventSingleTestCases); i++) {
                    int32_t tmpCount = 0;
                    _rc = (this->*(gEventSingleTestCases[i]))(&tmpCount);
                    if (!SUCCEED(_rc)) {
                        LOGE(mModule, "Failed to run event single test cases,"
                            " round %d, %d", i, _rc);
                    }
                    *count += tmpCount;
                }
                sem.signal();
                return _rc;
            }
        );
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to run all single test cases async, %d", rc);
            final |= rc;
            RESETRESULT(rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = mThreads->run(
            [&]() -> int32_t {
                int32_t _rc = NO_ERROR;
                for (uint32_t i = 0; i < gCustomDataSingleTestCaseSize; i++) {
                    int32_t tmpCount = 0;
                    _rc = (this->*(gCustomDataSingleTestCases[i]))(&tmpCount);
                    if (!SUCCEED(_rc)) {
                        LOGE(mModule, "Failed to run custom data single test cases,"
                            " round %d, %d", i, _rc);
                    }
                    *count += tmpCount;
                }
                sem.signal();
                return _rc;
            }
        );
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to run all single test cases async, %d", rc);
            final |= rc;
            RESETRESULT(rc);
        }
    }

    if (SUCCEED(final)) {
        for (int32_t i = 0; i < REQUEST_TYPE_MAX_INVALID; i++) {
            sem.wait();
        }
    }

    if (!SUCCEED(rc)) {
        LOGE(mModule, "Failed to run mixture test round %d, %d"
            "check previous logs", 1);
    } else {
        LOGI(mModule, "**************************************************");
        LOGI(mModule, "*        Run mixture test round 1 succeed       *");
        LOGI(mModule, "**************************************************\n");
    }

    return rc;
}

int32_t ClientTesterImpl::testMixtureRound2(int32_t *count)
{
    int32_t rc = NO_ERROR;
    int32_t final = rc;
    Semaphore sem;

    if (SUCCEED(rc)) {
        rc = mThreads->run(
            [&]() -> int32_t {
                int32_t _rc = NO_ERROR;
                for (int32_t i = ARRAYSIZE(gPreviewSingleTestCases) - 1; i >= 0 ; i--) {
                    int32_t tmpCount = 0;
                    _rc = (this->*(gPreviewSingleTestCases[i]))(&tmpCount);
                    if (!SUCCEED(_rc)) {
                        LOGE(mModule, "Failed to run preview single test cases,"
                            " round %d, %d", i, _rc);
                    }
                    *count += tmpCount;
                }
                sem.signal();
                return _rc;
            }
        );
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to run all single test cases async, %d", rc);
            final |= rc;
            RESETRESULT(rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = mThreads->run(
            [&]() -> int32_t {
                int32_t _rc = NO_ERROR;
                for (int32_t i = ARRAYSIZE(gNV21PictureSingleTestCases) - 1; i >=0 ; i--) {
                    int32_t tmpCount = 0;
                    _rc = (this->*(gNV21PictureSingleTestCases[i]))(&tmpCount);
                    if (!SUCCEED(_rc)) {
                        LOGE(mModule, "Failed to run nv21 picture single test cases,"
                            " round %d, %d", i, _rc);
                    }
                    *count += tmpCount;
                }
                sem.signal();
                return _rc;
            }
        );
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to run all single test cases async, %d", rc);
            final |= rc;
            RESETRESULT(rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = mThreads->run(
            [&]() -> int32_t {
                int32_t _rc = NO_ERROR;
                for (int32_t i = ARRAYSIZE(gBayerPictureSingleTestCases) - 1; i >= 0 ; i--) {
                    int32_t tmpCount = 0;
                    _rc = (this->*(gBayerPictureSingleTestCases[i]))(&tmpCount);
                    if (!SUCCEED(_rc)) {
                        LOGE(mModule, "Failed to run bayer picture single test cases,"
                            " round %d, %d", i, _rc);
                    }
                    *count += tmpCount;
                }
                sem.signal();
                return _rc;
            }
        );
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to run all single test cases async, %d", rc);
            final |= rc;
            RESETRESULT(rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = mThreads->run(
            [&]() -> int32_t {
                int32_t _rc = NO_ERROR;
                for (int32_t i = ARRAYSIZE(gEventSingleTestCases) - 1; i >= 0 ; i--) {
                    int32_t tmpCount = 0;
                    _rc = (this->*(gEventSingleTestCases[i]))(&tmpCount);
                    if (!SUCCEED(_rc)) {
                        LOGE(mModule, "Failed to run event single test cases,"
                            " round %d, %d", i, _rc);
                    }
                    *count += tmpCount;
                }
                sem.signal();
                return _rc;
            }
        );
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to run all single test cases async, %d", rc);
            final |= rc;
            RESETRESULT(rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = mThreads->run(
            [&]() -> int32_t {
                int32_t _rc = NO_ERROR;
                for (int32_t i = gCustomDataSingleTestCaseSize - 1; i >= 0 ; i--) {
                    int32_t tmpCount = 0;
                    _rc = (this->*(gCustomDataSingleTestCases[i]))(&tmpCount);
                    if (!SUCCEED(_rc)) {
                        LOGE(mModule, "Failed to run custom data single test cases,"
                            " round %d, %d", i, _rc);
                    }
                    *count += tmpCount;
                }
                sem.signal();
                return _rc;
            }
        );
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to run all single test cases async, %d", rc);
            final |= rc;
            RESETRESULT(rc);
        }
    }

    if (SUCCEED(final)) {
        for (int32_t i = 0; i < REQUEST_TYPE_MAX_INVALID; i++) {
            sem.wait();
        }
    }

    if (!SUCCEED(final)) {
        LOGE(mModule, "Failed to run mixture test round %d, %d"
            "check previous logs", 2);
    } else {
        LOGI(mModule, "************************************************");
        LOGI(mModule, "*        Run mixture test round 2 succeed       *");
        LOGI(mModule, "*************************************************\n");
    }

    return rc;
}

int32_t ClientTesterImpl::testMixtureRound3(int32_t *count)
{
    int32_t rc = NO_ERROR;
    int32_t final = rc;
    Semaphore sem;

    for (int32_t i = 0; i < ROUND3_TEST_COUNT; i++) {
        int32_t tmpCount = 0;
        rc = mThreads->run(
            [&]() -> int32_t {
                int32_t _rc = NO_ERROR;
                _rc = i % 2 ?
                    testMixtureRound1(&tmpCount) :
                    testMixtureRound2(&tmpCount);
                if (!SUCCEED(_rc)) {
                    LOGE(mModule, "Failed to run mixture test %d, %d", i, _rc);
                }
                *count += tmpCount;
                sem.signal();
                return _rc;
            }
        );
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to run all single test cases async, %d", rc);
            final |= rc;
            RESETRESULT(rc);
        }
    }

    for (int32_t i = 0; i < ROUND3_TEST_COUNT; i++) {
        sem.wait();
    }

    if (!SUCCEED(rc)) {
        LOGE(mModule, "Failed to run mixture test round %d, %d"
            "check previous logs", 3);
    } else {
        LOGI(mModule, "************************************************");
        LOGI(mModule, "*        Run mixture test round 3 succeed       *");
        LOGI(mModule, "*************************************************\n");
    }

    return rc;
}

int32_t ServerTesterImpl::cbMixtureRequest(
    RequestType type, int32_t id, void *header, void *dat)
{
    int32_t rc = NO_ERROR;
    int32_t logoId = 0;
    uint8_t *logo = NULL;
    int32_t size = 0;
    int32_t w, h, stride, scanline;
    int32_t headerW, headerH, headerSize;

    if (SUCCEED(rc)) {
        switch (type) {
            case PREVIEW_NV21: {
                logoId = 0;
                PreviewNV21Header *_header =
                    static_cast<PreviewNV21Header *>(header);
                headerW = _header->w;
                headerH = _header->h;
                headerSize = _header->size;
            }; break;
            case PICTURE_NV21: {
                logoId = 1;
                PictureNV21Header *_header =
                    static_cast<PictureNV21Header *>(header);
                headerW = _header->w;
                headerH = _header->h;
                headerSize = _header->size;
            }; break;
            case PICTURE_BAYER: {
                logoId = 3;
                PictureBayerHeader *_header =
                    static_cast<PictureBayerHeader *>(header);
                headerW = _header->w;
                headerH = _header->h;
                headerSize = _header->size;
            }; break;
            case EXTENDED_EVENT:
            case CUSTOM_DATA:
            case REQUEST_TYPE_MAX_INVALID: {
                LOGE(mModule, "Invalid cb handler, %s", getRequestName(type));
                logoId = 0;
                headerW = 0;
                headerH = 0;
                headerSize = 0;
                rc = PARAM_INVALID;
            }; break;
        }
    }

    if (SUCCEED(rc)) {
        rc = loadLogo(&logo, &size, logoId);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to load logo");
        }
    }

    if (SUCCEED(rc)) {
        rc = getLogoSize(logoId, &w, &h, &stride, &scanline);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to get yuv logo size, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        if (w != headerW ||
            h != headerH ||
            headerSize != size) {
            LOGE(mModule, "Corrupted %s request data, "
                "id %d size mismatch, server %dx%d=%d, "
                "client %dx%d=%d", getRequestName(type), id,
                headerW, headerH, headerSize, w, h, size);
            rc = TEST_FAILED;
        }
    }

    if (SUCCEED(rc)) {
        uint8_t *data = (uint8_t *)dat;
        int64_t hash1 = hashLogo(logo, size);
        int64_t hash2 = hashLogo(data, headerSize);
        if (hash1 != hash2) {
            LOGE(mModule, " Corrupted %s request data id %d Hash "
                "data mismatch, size1 %d size2 %d hash1 %llu hash2 %llu",
                getRequestName(type), id, size, headerSize, hash1, hash2);
            rc = TEST_FAILED;
        }
    }

    if (SUCCEED(rc)){
        LOGD(mModule, "Receievd %s cb on server and checked alright "
            "in mixture test. <-", getRequestName(type));
    } else {
        LOGE(mModule, "Received %s but error detected, check previous"
            " log.", getRequestName(type));
    }

    SECURE_FREE(logo);

    return rc;
}

int32_t ServerTesterImpl::cbMixtureEvent(int32_t event, int32_t arg1, int32_t arg2)
{
    int32_t rc = NO_ERROR;

    if (SUCCEED(rc)) {
        if (event != arg1 - 1 ||
            event + arg1 != arg2) {
            LOGE(mModule, "Corrupted event received in event [%d %d %d]",
                event, arg1, arg2);
            rc = TEST_FAILED;
        }
    }

    if (SUCCEED(rc)){
        LOGD(mModule, "Receievd event on server and checked alright."
            " Event [%d %d %d] <-", event, arg1, arg2);
    } else {
        LOGE(mModule, "Received event but error detected.");
    }

    return rc;
}

int32_t ServerTesterImpl::cbMixtureData(int32_t type, void *_data, int32_t size)
{
    int32_t rc = NO_ERROR;

    if (SUCCEED(rc)) {
        rc = findMatchTypeSize(type, size);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Not found matched type %d and size %d",
                type, size);
        }
    }

    if (SUCCEED(rc)) {
        uint8_t *data = (uint8_t *)_data;
        int8_t diff = data[1] - data[0];
        diff = diff > 0 ? diff : diff + 0xFF;
        for (int32_t i = 1; i < size; i++) {
            if ((data[i - 1] + diff) % 0xFF != data[i]) {
                LOGE(mModule, "Failed to verify data for type %d size %d",
                    type, size);
                rc = TEST_FAILED;
                break;
            }
        }
    }

    if (SUCCEED(rc)){
        LOGD(mModule, "Receievd custom data on server and checked alright."
            "type %d size %d <-", type, size);
    } else {
        LOGE(mModule, "Received custom data but error detected."
            "type %d size %d !", type, size);
    }

    return rc;
}

};

