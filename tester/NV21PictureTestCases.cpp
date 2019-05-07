#include "common.h"
#include "TimeEx.h"
#include "ClientTesterImpl.h"
#include "ServerTesterImpl.h"
#include "YuvPictureOperator.h"

#define USE_YUV_LOGO_TYPE 1

#define ROUND1_TEST_DELAY_PER_FRAME 300 //ms
#define ROUND1_TEST_COUNT (1000 / ROUND1_TEST_DELAY_PER_FRAME * 15) // test 10 seconds
#define ROUND2_TEST_DELAY_PER_FRAME 100
#define ROUND2_TEST_COUNT (1000 / ROUND2_TEST_DELAY_PER_FRAME * 10)
#define ROUND3_TEST_DELAY_PER_FRAME 30
#define ROUND3_TEST_COUNT (1000 / ROUND3_TEST_DELAY_PER_FRAME * 10)
#define ROUND4_TEST_DELAY_PER_FRAME 10
#define ROUND4_TEST_COUNT (1000 / ROUND4_TEST_DELAY_PER_FRAME * 5)
#define ROUND5_TEST_DELAY_PER_FRAME 1
#define ROUND5_TEST_COUNT (1000 / ROUND5_TEST_DELAY_PER_FRAME * 5)
#define ROUND6_MULTI_THREAD_NUM     2  // Multi-thread test
#define ROUND6_TEST_DELAY_PER_FRAME 100
#define ROUND6_TEST_COUNT (1000 / ROUND6_TEST_DELAY_PER_FRAME * 10)
#define ROUND7_MULTI_THREAD_NUM     5
#define ROUND7_TEST_DELAY_PER_FRAME 30
#define ROUND7_TEST_COUNT (1000 / ROUND7_TEST_DELAY_PER_FRAME * 8)

namespace tester {

using namespace sirius;

int32_t ClientTesterImpl::testNV21PictureRound(
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
                rc = mSirius.onYuvPictureReady(w, h,
                    stride, scanline, logo, currentUs());
                if (!SUCCEED(rc)) {
                    LOGE(mModule, "Failed to send nv21 picture ready to sirius"
                        " server, round %d frame %d, %d", round, i + 1, rc);
                } else {
                    LOGI(mModule, "NV21 picture Test, client round %d, sent %d "
                        "nv21 picture frame to server succeed. ->", round, i + 1);
                }
            }

            RESETRESULT(rc);
            delayMs(msTime);
        }
    }

    SECURE_FREE(logo);

    return rc;
}

int32_t ServerTesterImpl::cbNV21PictureRound(
    int32_t round, int32_t id, PictureNV21Header *header, uint8_t *dat)
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
            LOGE(mModule, "Corrupted nv21 picture request data, "
                "round %d id %d frame %d Size mismatch, server "
                "%dx%d=%d, client %dx%d=%d", round, id, mNV21PictureCnt,
                header->w, header->h, header->size, w, h, size);
            rc = TEST_FAILED;
        }
    }

    if (SUCCEED(rc)) {
        int64_t hash1 = hashLogo(logo, size);
        int64_t hash2 = hashLogo(dat, header->size);
        if (hash1 != hash2) {
            LOGE(mModule, "Corrupted nv21 picture request data, "
                "round %d id %d frame %d Hash data mismatch, %ll %ll",
                round, id, mNV21PictureCnt, hash1, hash2);
            rc = TEST_FAILED;
        }
    }

    if (SUCCEED(rc)) {
        uint8_t identity = 0;
        rc = getLogoIndentifier(dat, size, &identity);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to get yuv idnetifier, %d", rc);
        } else {
            if (mNV21PictureIdentity > identity && identity != 0) {
                LOGW(mModule, "Wrong nv21 picture order detected."
                    "Round %d id %d frame %d", round, id, mNV21PictureCnt);
                //rc = TEST_FAILED;
            } else {
                mNV21PictureIdentity = identity;
            }
        }
    }

    if (SUCCEED(rc)){
        LOGI(mModule, "Receievd nv21 picture cb on server and"
            " checked alright. Round %d id %d frame %d <-",
            round, id, mNV21PictureCnt);
    } else {
        LOGE(mModule, "Received nv21 picture but error detected, "
            "check previous log. Round %d id %d frame %d.",
            round, id, mNV21PictureCnt);
    }

    SECURE_FREE(logo);

    return rc;
}

int32_t ClientTesterImpl::testNV21PictureRound1(int32_t *count)
{
    return testNV21PictureRound(1, ROUND1_TEST_COUNT, ROUND1_TEST_DELAY_PER_FRAME, count);
}

int32_t ServerTesterImpl::cbNV21PictureRound1(
    int32_t id, PictureNV21Header *header, uint8_t *dat)
{
    return cbNV21PictureRound(1, id, header, dat);
}

int32_t ClientTesterImpl::testNV21PictureRound2(int32_t *count)
{
    return testNV21PictureRound(2, ROUND2_TEST_COUNT, ROUND2_TEST_DELAY_PER_FRAME, count);
}

int32_t ServerTesterImpl::cbNV21PictureRound2(
    int32_t id, PictureNV21Header *header, uint8_t *dat)
{
    return cbNV21PictureRound(2, id, header, dat);
}

int32_t ClientTesterImpl::testNV21PictureRound3(int32_t *count)
{
    return testNV21PictureRound(3, ROUND3_TEST_COUNT, ROUND3_TEST_DELAY_PER_FRAME, count);
}

int32_t ServerTesterImpl::cbNV21PictureRound3(
    int32_t id, PictureNV21Header *header, uint8_t *dat)
{
    return cbNV21PictureRound(3, id, header, dat);
}

int32_t ClientTesterImpl::testNV21PictureRound4(int32_t *count)
{
    return testNV21PictureRound(4, ROUND4_TEST_COUNT, ROUND4_TEST_DELAY_PER_FRAME, count);
}

int32_t ServerTesterImpl::cbNV21PictureRound4(
    int32_t id, PictureNV21Header *header, uint8_t *dat)
{
    return cbNV21PictureRound(4, id, header, dat);
}

int32_t ClientTesterImpl::testNV21PictureRound5(int32_t *count)
{
    return testNV21PictureRound(5, ROUND5_TEST_COUNT, ROUND5_TEST_DELAY_PER_FRAME, count);
}

int32_t ServerTesterImpl::cbNV21PictureRound5(
    int32_t id, PictureNV21Header *header, uint8_t *dat)
{
    return cbNV21PictureRound(5, id, header, dat);
}

int32_t ClientTesterImpl::testNV21PictureMultiThread(
    int32_t threadNum, int32_t round, int32_t targetCount, int32_t msTime, int32_t *count)
{
    int32_t rc = NO_ERROR;
    Semaphore sem;

    if (SUCCEED(rc)) {
        for (int32_t i = 0; i < threadNum; i++) {
            rc = mThreads->run(
                [&]() -> int32_t {
                    int32_t _rc = testNV21PictureRound(round, targetCount, msTime, count);
                    sem.signal();
                    if (!SUCCEED(_rc)) {
                        LOGE(mModule, "Failed to run nv picture round %d", round);
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

int32_t ClientTesterImpl::testNV21PictureRound6(int32_t *count)
{
    return testNV21PictureMultiThread(ROUND6_MULTI_THREAD_NUM,
        6, ROUND6_TEST_COUNT, ROUND6_TEST_DELAY_PER_FRAME, count);
}

int32_t ServerTesterImpl::cbNV21PictureRound6(
    int32_t id, PictureNV21Header *header, uint8_t *dat)
{
    return cbNV21PictureRound(6, id, header, dat);
}

int32_t ClientTesterImpl::testNV21PictureRound7(int32_t *count)
{
    return testNV21PictureMultiThread(ROUND7_MULTI_THREAD_NUM,
        7, ROUND7_TEST_COUNT, ROUND7_TEST_DELAY_PER_FRAME, count);
}

int32_t ServerTesterImpl::cbNV21PictureRound7(
    int32_t id, PictureNV21Header *header, uint8_t *dat)
{
    return cbNV21PictureRound(7, id, header, dat);
}

};

