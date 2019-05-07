#include "common.h"
#include "ClientTesterImpl.h"
#include "ServerTesterImpl.h"

#define ROUND1_TEST_DELAY_PER_FRAME 100 //ms
#define ROUND1_TEST_COUNT (1000 / ROUND1_TEST_DELAY_PER_FRAME * 10) // test 15 seconds
#define ROUND2_TEST_DELAY_PER_FRAME 10
#define ROUND2_TEST_COUNT (1000 / ROUND2_TEST_DELAY_PER_FRAME * 5)
#define ROUND3_TEST_DELAY_PER_FRAME 5
#define ROUND3_TEST_COUNT (1000 / ROUND3_TEST_DELAY_PER_FRAME * 5)
#define ROUND4_TEST_DELAY_PER_FRAME 2
#define ROUND4_TEST_COUNT (1000 / ROUND4_TEST_DELAY_PER_FRAME * 3)
#define ROUND5_MULTI_THREAD_NUM     2  // Multi-thread test
#define ROUND5_TEST_DELAY_PER_FRAME 5
#define ROUND5_TEST_COUNT (1000 / ROUND5_TEST_DELAY_PER_FRAME * 5)
#define ROUND6_MULTI_THREAD_NUM     4
#define ROUND6_TEST_DELAY_PER_FRAME 12
#define ROUND6_TEST_COUNT (1000 / ROUND6_TEST_DELAY_PER_FRAME * 5)
#define ROUND7_MULTI_THREAD_NUM     8
#define ROUND7_TEST_DELAY_PER_FRAME 25
#define ROUND7_TEST_COUNT (1000 / ROUND7_TEST_DELAY_PER_FRAME * 5)

namespace tester {

using namespace sirius;

int32_t ClientTesterImpl::testEventsRound(
    int32_t round, int32_t targetCount, int32_t msTime, int32_t *count)
{
    int32_t rc = NO_ERROR;
    int32_t final = rc;
    *count = targetCount;

    for (int32_t i = 0; i < *count; i++) {
        rc = mSirius.sendEvent(i, i + 1, i + i + 1);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to send event to sirius "
                "server, round %d frame %d, %d", round, i + 1, rc);
            final |= rc;
        } else {
            LOGI(mModule, "Event Test, client round %d, sent %d "
                "event to server succeed. ->", round, i + 1);
        }

        RESETRESULT(rc);
        delayMs(msTime);
    }

    return final;
}

int32_t ServerTesterImpl::cbEventRound(int32_t round, int32_t event, int32_t arg1, int32_t arg2)
{
    int32_t rc = NO_ERROR;

    if (SUCCEED(rc)) {
        if (event != arg1 - 1 ||
            event + arg1 != arg2) {
            LOGE(mModule, "Corrupted event received in "
                "round %d, event id %d, [%d %d %d]",
                round, mEventCnt, event, arg1, arg2);
            rc = TEST_FAILED;
        }
    }


    if (SUCCEED(rc)) {
        if (mLastEvent > event) {
            LOGW(mModule, "Wrong event order detected."
                "Round %d last event %d this event %d",
                round, mLastEvent, event);
            //rc = TEST_FAILED;
        } else {
            mLastEvent = event;
        }
    }

    if (SUCCEED(rc)){
        LOGI(mModule, "Receievd event on server and checked alright."
            " Round %d event %d <-", round, event);
    } else {
        LOGE(mModule, "Received event but error detected, "
            "check previous log. Round %d event %d <-", round, event);
    }

    return rc;
}

int32_t ClientTesterImpl::testEventsRound1(int32_t *count)
{
    return testEventsRound(1, ROUND1_TEST_COUNT, ROUND1_TEST_DELAY_PER_FRAME, count);
}

int32_t ServerTesterImpl::cbEventRound1(int32_t event, int32_t arg1, int32_t arg2)
{
    return cbEventRound(1, event, arg1, arg2);
}

int32_t ClientTesterImpl::testEventsRound2(int32_t *count)
{
    return testEventsRound(2, ROUND2_TEST_COUNT, ROUND2_TEST_DELAY_PER_FRAME, count);
}

int32_t ServerTesterImpl::cbEventRound2(int32_t event, int32_t arg1, int32_t arg2)
{
    return cbEventRound(2, event, arg1, arg2);
}

int32_t ClientTesterImpl::testEventsRound3(int32_t *count)
{
    return testEventsRound(3, ROUND3_TEST_COUNT, ROUND3_TEST_DELAY_PER_FRAME, count);
}

int32_t ServerTesterImpl::cbEventRound3(int32_t event, int32_t arg1, int32_t arg2)
{
    return cbEventRound(3, event, arg1, arg2);
}

int32_t ClientTesterImpl::testEventsRound4(int32_t *count)
{
    return testEventsRound(4, ROUND4_TEST_COUNT, ROUND4_TEST_DELAY_PER_FRAME, count);
}

int32_t ServerTesterImpl::cbEventRound4(int32_t event, int32_t arg1, int32_t arg2)
{
    return cbEventRound(4, event, arg1, arg2);
}

int32_t ClientTesterImpl::testEventsMultiThread(
    int32_t threadNum, int32_t round, int32_t targetCount, int32_t msTime, int32_t *count)
{
    int32_t rc = NO_ERROR;
    Semaphore sem;

    if (SUCCEED(rc)) {
        for (int32_t i = 0; i < threadNum; i++) {
            rc = mThreads->run(
                [&]() -> int32_t {
                    int32_t _rc = testEventsRound(round, targetCount, msTime, count);
                    sem.signal();
                    if (!SUCCEED(_rc)) {
                        LOGE(mModule, "Failed to run events round %d", round);
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

int32_t ClientTesterImpl::testEventsRound5(int32_t *count)
{
    return testEventsMultiThread(ROUND5_MULTI_THREAD_NUM,
        5, ROUND5_TEST_COUNT, ROUND5_TEST_DELAY_PER_FRAME, count);
}

int32_t ServerTesterImpl::cbEventRound5(int32_t event, int32_t arg1, int32_t arg2)
{
    return cbEventRound(5, event, arg1, arg2);
}

int32_t ClientTesterImpl::testEventsRound6(int32_t *count)
{
    return testEventsMultiThread(ROUND6_MULTI_THREAD_NUM,
        6, ROUND6_TEST_COUNT, ROUND6_TEST_DELAY_PER_FRAME, count);
}

int32_t ServerTesterImpl::cbEventRound6(int32_t event, int32_t arg1, int32_t arg2)
{
    return cbEventRound(6, event, arg1, arg2);
}

int32_t ClientTesterImpl::testEventsRound7(int32_t *count)
{
    return testEventsMultiThread(ROUND7_MULTI_THREAD_NUM,
        7, ROUND7_TEST_COUNT, ROUND7_TEST_DELAY_PER_FRAME, count);
}

int32_t ServerTesterImpl::cbEventRound7(int32_t event, int32_t arg1, int32_t arg2)
{
    return cbEventRound(7, event, arg1, arg2);
}

};

