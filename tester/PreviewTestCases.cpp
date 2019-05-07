#include "common.h"
#include "TimeEx.h"
#include "ClientTesterImpl.h"
#include "ServerTesterImpl.h"
#include "YuvPictureOperator.h"

#define USE_YUV_LOGO_TYPE 0

#define ROUND1_TEST_DELAY_PER_FRAME 100 //ms
#define ROUND1_TEST_COUNT (1000 / ROUND1_TEST_DELAY_PER_FRAME * 15) // test 15 seconds

#define ROUND2_TEST_DELAY_PER_FRAME 30
#define ROUND2_TEST_COUNT (1000 / ROUND2_TEST_DELAY_PER_FRAME * 15)

#define ROUND3_TEST_DELAY_PER_FRAME 15
#define ROUND3_TEST_COUNT (1000 / ROUND3_TEST_DELAY_PER_FRAME * 10)

#define ROUND4_TEST_DELAY_PER_FRAME 5
#define ROUND4_TEST_COUNT (1000 / ROUND4_TEST_DELAY_PER_FRAME * 10)

#define ROUND5_TEST_DELAY_PER_FRAME 1
#define ROUND5_TEST_COUNT (1000 / ROUND5_TEST_DELAY_PER_FRAME * 5)

#define ROUND6_MULTI_THREAD_NUM     2  // Multi-thread test
#define ROUND6_TEST_DELAY_PER_FRAME 30
#define ROUND6_TEST_COUNT (1000 / ROUND6_TEST_DELAY_PER_FRAME * 15)

#define ROUND7_MULTI_THREAD_NUM     5
#define ROUND7_TEST_DELAY_PER_FRAME 30
#define ROUND7_TEST_COUNT (1000 / ROUND7_TEST_DELAY_PER_FRAME * 15)

namespace tester {

using namespace sirius;

int32_t ClientTesterImpl::testPreviewRound(
    int32_t round, int32_t targetCount, int32_t msTime, int32_t *count)
{
    int32_t rc = NO_ERROR;
    uint8_t *logo = NULL;
    int32_t size = 0;
    int32_t w, h, stride, scanline;

    if (SUCCEED(rc)) {
        *count = targetCount;
        rc = loadLogo(&logo, &size, USE_YUV_LOGO_TYPE);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to load logo");
        }
    }

    if (SUCCEED(rc)) {
        rc = getLogoSize(USE_YUV_LOGO_TYPE, &w, &h, &stride, &scanline);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to get yuv logo size, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        for (int32_t i = 0; i < *count; i++) {
            if (SUCCEED(rc)) {
                rc = setLogoIndentifier(logo, size, i % 0xFF);
                if (!SUCCEED(rc)) {
                    LOGE(mModule, "Failed to set indentifier %d, %d", i, rc);
                }
            }

            if (SUCCEED(rc)) {
                rc = mSirius.onPreviewReady(w, h,
                    stride, scanline, logo, currentUs());
                if (!SUCCEED(rc)) {
                    LOGE(mModule, "Failed to send preview ready to sirius "
                        "server, round %d frame %d, %d", round, i + 1, rc);
                } else {
                    LOGI(mModule, "Preview Request Test, Round %d, sent %d "
                        "preview frame to server succeed. ->", round, i + 1);
                }
            }

            RESETRESULT(rc);
            delayMs(msTime);
        }
    }

    SECURE_FREE(logo);

    return rc;
}

int32_t ServerTesterImpl::cbPreviewRound(
    int32_t round, int32_t id, PreviewNV21Header *header, uint8_t *dat)
{
    int32_t rc = NO_ERROR;
    uint8_t *logo = NULL;
    int32_t size = 0;
    int32_t w, h, stride, scanline;

    if (SUCCEED(rc)) {
        rc = loadLogo(&logo, &size, USE_YUV_LOGO_TYPE);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to load logo");
        }
    }

    if (SUCCEED(rc)) {
        rc = getLogoSize(USE_YUV_LOGO_TYPE, &w, &h, &stride, &scanline);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to get yuv logo size, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        if (w != header->w ||
            h != header->h ||
            header->size != size) {
            LOGE(mModule, "Corrupted preview request data, "
                "round %d id %d frame %d Size mismatch, server "
                "%dx%d=%d, client %dx%d=%d", round, id, mPreviewCnt,
                header->w, header->h, header->size, w, h, size);
            rc = TEST_FAILED;
        }
    }

    if (SUCCEED(rc)) {
        int64_t hash1 = hashLogo(logo, size);
        int64_t hash2 = hashLogo(dat, header->size);
        if (hash1 != hash2) {
            LOGE(mModule, " Corrupted preview request data round %d"
                " id %d frame %d Hash data mismatch,"
                "size1 %d size2 %d hash1 %llu hash2 %llu",
                round, id, mPreviewCnt, size, header->size, hash1, hash2);
            rc = TEST_FAILED;
        }
    }

    if (SUCCEED(rc)) {
        uint8_t identity = 0;
        rc = getLogoIndentifier(dat, size, &identity);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to get yuv idnetifier, %d", rc);
        } else {
            if (mPreviewIdentity > identity && identity != 0) {
                LOGW(mModule, "Wrong preview frame order detected."
                    "Round %d id %d frame %d", round, id, mPreviewCnt);
                //rc = TEST_FAILED;
            } else {
                mPreviewIdentity = identity;
            }
        }
    }

    if (SUCCEED(rc)){
        LOGI(mModule, "Receievd preview cb on server and "
            "checked alright. Round %d id %d frame %d <-",
            round, id, mPreviewCnt);
    } else {
        LOGE(mModule, "Received preview but error detected, "
            "check previous log. Round %d id %d frame %d.",
            round, id, mPreviewCnt);
    }

    SECURE_FREE(logo);

    return rc;
}

int32_t ClientTesterImpl::testPreviewRound1(int32_t *count)
{
    return testPreviewRound(1, ROUND1_TEST_COUNT, ROUND1_TEST_DELAY_PER_FRAME, count);
}

int32_t ServerTesterImpl::cbPreviewRound1(
    int32_t id, PreviewNV21Header *header, uint8_t *dat)
{
    return cbPreviewRound(1, id, header, dat);
}

int32_t ClientTesterImpl::testPreviewRound2(int32_t *count)
{
    return testPreviewRound(2, ROUND2_TEST_COUNT, ROUND2_TEST_DELAY_PER_FRAME, count);
}

int32_t ServerTesterImpl::cbPreviewRound2(
    int32_t id, PreviewNV21Header *header, uint8_t *dat)
{
    return cbPreviewRound(2, id, header, dat);
}

int32_t ClientTesterImpl::testPreviewRound3(int32_t *count)
{
    return testPreviewRound(3, ROUND3_TEST_COUNT, ROUND3_TEST_DELAY_PER_FRAME, count);
}

int32_t ServerTesterImpl::cbPreviewRound3(
    int32_t id, PreviewNV21Header *header, uint8_t *dat)
{
    return cbPreviewRound(3, id, header, dat);
}

int32_t ClientTesterImpl::testPreviewRound4(int32_t *count)
{
    return testPreviewRound(4, ROUND4_TEST_COUNT, ROUND4_TEST_DELAY_PER_FRAME, count);
}

int32_t ServerTesterImpl::cbPreviewRound4(
    int32_t id, PreviewNV21Header *header, uint8_t *dat)
{
    return cbPreviewRound(4, id, header, dat);
}

int32_t ClientTesterImpl::testPreviewRound5(int32_t *count)
{
    return testPreviewRound(5, ROUND5_TEST_COUNT, ROUND5_TEST_DELAY_PER_FRAME, count);
}

int32_t ServerTesterImpl::cbPreviewRound5(
    int32_t id, PreviewNV21Header *header, uint8_t *dat)
{
    return cbPreviewRound(5, id, header, dat);
}

int32_t ClientTesterImpl::testPreviewMultiThread(
    int32_t threadNum, int32_t round, int32_t targetCount, int32_t msTime, int32_t *count)
{
    int32_t rc = NO_ERROR;
    Semaphore sem;

    if (SUCCEED(rc)) {
        for (int32_t i = 0; i < threadNum; i++) {
            rc = mThreads->run(
                [&]() -> int32_t {
                    int32_t _rc = testPreviewRound(round, targetCount, msTime, count);
                    sem.signal();
                    if (!SUCCEED(_rc)) {
                        LOGE(mModule, "Failed to run preview round %d", round);
                    }
                    return _rc;
                }
            );
            if (!SUCCEED(rc)) {
                LOGE(mModule, "Failed to run on thread, %d", rc);
            }
            RESETRESULT(rc);
        }
    }

    if (SUCCEED(rc)) {
        for (int32_t i = 0; i < threadNum; i++) {
            // Wait all threads to finish task
            sem.wait();
        }
    }

    if (SUCCEED(rc) || !SUCCEED(rc)) {
        *count = threadNum * targetCount;
    }

    return rc;
}

int32_t ClientTesterImpl::testPreviewRound6(int32_t *count)
{
    return testPreviewMultiThread(ROUND6_MULTI_THREAD_NUM,
        6, ROUND6_TEST_COUNT, ROUND6_TEST_DELAY_PER_FRAME, count);
}

int32_t ServerTesterImpl::cbPreviewRound6(
    int32_t id, PreviewNV21Header *header, uint8_t *dat)
{
    return cbPreviewRound(6, id, header, dat);
}

int32_t ClientTesterImpl::testPreviewRound7(int32_t *count)
{
    return testPreviewMultiThread(ROUND7_MULTI_THREAD_NUM,
        7, ROUND7_TEST_COUNT, ROUND7_TEST_DELAY_PER_FRAME, count);
}

int32_t ServerTesterImpl::cbPreviewRound7(
    int32_t id, PreviewNV21Header *header, uint8_t *dat)
{
    return cbPreviewRound(7, id, header, dat);
}

};

