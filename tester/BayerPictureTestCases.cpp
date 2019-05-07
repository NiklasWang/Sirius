#include "common.h"
#include "TimeEx.h"
#include "ClientTesterImpl.h"
#include "ServerTesterImpl.h"
#include "YuvPictureOperator.h"

#define USE_RAW_DATA_TYPE 3

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

int32_t ClientTesterImpl::testBayerPictureRound(
    int32_t round, int32_t targetCount, int32_t msTime, int32_t *count)
{
    int32_t rc = NO_ERROR;
    uint8_t *logo = NULL;
    int32_t size = 0;
    int32_t w, h, stride, scanline;

    if (SUCCEED(rc)) {
        *count = targetCount;
        rc = loadLogo(&logo, &size, USE_RAW_DATA_TYPE);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to load logo");
        }
    }

    if (SUCCEED(rc)) {
        rc = getLogoSize(USE_RAW_DATA_TYPE, &w, &h, &stride, &scanline);
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
                rc = mSirius.onBayerPictureReady(w, h,
                    logo, currentUs(), BAYER_PATTERN_RGGB);
                if (!SUCCEED(rc)) {
                    LOGE(mModule, "Failed to send bayer picture ready to sirius"
                        " server, round %d frame %d, %d", round, i + 1, rc);
                } else {
                    LOGI(mModule, "Bayer picture Test, client round %d, sent %d "
                        "bayer picture to server succeed. ->", round, i + 1);
                }
            }

            RESETRESULT(rc);
            delayMs(msTime);
        }
    }

    SECURE_FREE(logo);

    return rc;
}

int32_t ServerTesterImpl::cbBayerPictureRound(
    int32_t round, int32_t id, PictureBayerHeader *header, uint8_t *dat)
{
    int32_t rc = NO_ERROR;
    uint8_t *logo = NULL;
    int32_t size = 0;
    int32_t w, h, stride, scanline;

    if (SUCCEED(rc)) {
        rc = loadLogo(&logo, &size, USE_RAW_DATA_TYPE);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to load logo");
        }
    }

    if (SUCCEED(rc)) {
        rc = getLogoSize(USE_RAW_DATA_TYPE, &w, &h, &stride, &scanline);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to get yuv logo size, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        if (w != header->w ||
            h != header->h ||
            header->size != size) {
            LOGE(mModule, "Corrupted bayer picture request data, "
                "round %d id %d frame %d Size mismatch, server "
                "%dx%d=%d, client %dx%d=%d", round, id,
                mBayerPictureCnt, header->w, header->h,
                header->size, w, h, size);
            rc = TEST_FAILED;
        }
    }

    if (SUCCEED(rc)) {
        int64_t hash1 = hashLogo(logo, size);
        int64_t hash2 = hashLogo(dat, header->size);
        if (hash1 != hash2) {
            LOGE(mModule, "Corrupted bayer picture data, round %d"
                " id %d frame %d Hash data mismatch, %ll %ll",
                round, id, mBayerPictureCnt, hash1, hash2);
            rc = TEST_FAILED;
        }
    }

    if (SUCCEED(rc)) {
        uint8_t identity = 0;
        rc = getLogoIndentifier(dat, size, &identity);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to get yuv idnetifier, %d", rc);
        } else {
            if (mBayerPictureIdentity > identity && identity != 0) {
                LOGW(mModule, "Wrong bayer picture order detected."
                    "Round %d id %d frame %d", round, id, mBayerPictureCnt);
                //rc = TEST_FAILED;
            } else {
                mBayerPictureIdentity = identity;
            }
        }
    }

    if (SUCCEED(rc)){
        LOGI(mModule, "Receievd bayer picture cb on server "
            "and checked alright. Round %d id %d frame %d <-",
            round, id, mBayerPictureCnt);
    } else {
        LOGE(mModule, "Received bayer picture but error "
            "detected, check previous log. Round %d id %d "
            "frame %d.", round, id, mBayerPictureCnt);
    }

    SECURE_FREE(logo);

    return rc;
}

int32_t ClientTesterImpl::testBayerPictureRound1(int32_t *count)
{
    return testBayerPictureRound(1, ROUND1_TEST_COUNT, ROUND1_TEST_DELAY_PER_FRAME, count);
}

int32_t ServerTesterImpl::cbBayerPictureRound1(
    int32_t id, PictureBayerHeader *header, uint8_t *dat)
{
    return cbBayerPictureRound(1, id, header, dat);
}

int32_t ClientTesterImpl::testBayerPictureRound2(int32_t *count)
{
    return testBayerPictureRound(2, ROUND2_TEST_COUNT, ROUND2_TEST_DELAY_PER_FRAME, count);
}

int32_t ServerTesterImpl::cbBayerPictureRound2(
    int32_t id, PictureBayerHeader *header, uint8_t *dat)
{
    return cbBayerPictureRound(2, id, header, dat);
}

int32_t ClientTesterImpl::testBayerPictureRound3(int32_t *count)
{
    return testBayerPictureRound(3, ROUND3_TEST_COUNT, ROUND3_TEST_DELAY_PER_FRAME, count);
}

int32_t ServerTesterImpl::cbBayerPictureRound3(
    int32_t id, PictureBayerHeader *header, uint8_t *dat)
{
    return cbBayerPictureRound(3, id, header, dat);
}

int32_t ClientTesterImpl::testBayerPictureRound4(int32_t *count)
{
    return testBayerPictureRound(4, ROUND4_TEST_COUNT, ROUND4_TEST_DELAY_PER_FRAME, count);
}

int32_t ServerTesterImpl::cbBayerPictureRound4(
    int32_t id, PictureBayerHeader *header, uint8_t *dat)
{
    return cbBayerPictureRound(4, id, header, dat);
}

int32_t ClientTesterImpl::testBayerPictureRound5(int32_t *count)
{
    return testBayerPictureRound(5, ROUND5_TEST_COUNT, ROUND5_TEST_DELAY_PER_FRAME, count);
}

int32_t ServerTesterImpl::cbBayerPictureRound5(
    int32_t id, PictureBayerHeader *header, uint8_t *dat)
{
    return cbBayerPictureRound(5, id, header, dat);
}

int32_t ClientTesterImpl::testBayerPictureMultiThread(
    int32_t threadNum, int32_t round, int32_t targetCount, int32_t msTime, int32_t *count)
{
    int32_t rc = NO_ERROR;
    Semaphore sem;

    if (SUCCEED(rc)) {
        for (int32_t i = 0; i < threadNum; i++) {
            rc = mThreads->run(
                [&]() -> int32_t {
                    int32_t _rc = testBayerPictureRound(round, targetCount, msTime, count);
                    sem.signal();
                    if (!SUCCEED(_rc)) {
                        LOGE(mModule, "Failed to run bayer picture round %d", round);
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

int32_t ClientTesterImpl::testBayerPictureRound6(int32_t *count)
{
    return testBayerPictureMultiThread(ROUND6_MULTI_THREAD_NUM,
        6, ROUND6_TEST_COUNT, ROUND6_TEST_DELAY_PER_FRAME, count);
}

int32_t ServerTesterImpl::cbBayerPictureRound6(
    int32_t id, PictureBayerHeader *header, uint8_t *dat)
{
    return cbBayerPictureRound(6, id, header, dat);
}

int32_t ClientTesterImpl::testBayerPictureRound7(int32_t *count)
{
    return testBayerPictureMultiThread(ROUND7_MULTI_THREAD_NUM,
        7, ROUND7_TEST_COUNT, ROUND7_TEST_DELAY_PER_FRAME, count);
}

int32_t ServerTesterImpl::cbBayerPictureRound7(
    int32_t id, PictureBayerHeader *header, uint8_t *dat)
{
    return cbBayerPictureRound(7, id, header, dat);
}

};


