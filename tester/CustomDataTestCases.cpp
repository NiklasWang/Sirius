#include "common.h"
#include "ClientTesterImpl.h"
#include "ServerTesterImpl.h"
#include "DataRequestHeader.h"

#define ROUND1_DATA_SIZE 1 // bytes
#define ROUND2_DATA_SIZE 2
#define ROUND3_DATA_SIZE (DATA_TRANSMIT_MEMORY_SIZE / 2)
#define ROUND4_DATA_SIZE (DATA_TRANSMIT_MEMORY_SIZE - 1)
#define ROUND5_DATA_SIZE (DATA_TRANSMIT_MEMORY_SIZE)
#define ROUND6_DATA_SIZE (DATA_TRANSMIT_MEMORY_SIZE + 1)
#define ROUND7_DATA_SIZE (DATA_TRANSMIT_MEMORY_SIZE * CUSTOM_DATA_MEMORY_NUM - 1)
#define ROUND8_DATA_SIZE (DATA_TRANSMIT_MEMORY_SIZE * CUSTOM_DATA_MEMORY_NUM)
#define ROUND9_DATA_SIZE (DATA_TRANSMIT_MEMORY_SIZE * CUSTOM_DATA_MEMORY_NUM + 1)
#define ROUND10_DATA_SIZE (DATA_TRANSMIT_MEMORY_SIZE * CUSTOM_DATA_MEMORY_NUM * 3 - 1)
#define ROUND11_DATA_SIZE (DATA_TRANSMIT_MEMORY_SIZE * CUSTOM_DATA_MEMORY_NUM * 3)
#define ROUND12_DATA_SIZE (DATA_TRANSMIT_MEMORY_SIZE * CUSTOM_DATA_MEMORY_NUM * 3 + 1)

#define ROUND13_ONETHREAD_ROUND 5
#define ROUND13_ROUND_DELAY     10 // ms
#define ROUND14_ONETHREAD_ROUND 5
#define ROUND14_ROUND_DELAY     0

#define ROUND15_MULTITHREAD_NUM 4
#define ROUND15_LOOP_DELAY      5
#define ROUND15_CASE_DELAY      5

#define ROUND16_MULTITHREAD_NUM 4
#define ROUND16_LOOP_DELAY      0
#define ROUND16_CASE_DELAY      0

namespace tester {

using namespace sirius;

static const struct TestInfo {
    int32_t round;
    int32_t size;
    int32_t type;
    int32_t shuffle;
    int32_t count;
} gTestInfo[] = {
    [0] = {
        .round = 1,
        .size  = ROUND1_DATA_SIZE,
        .type  = 0xAD,
        .shuffle = 0,
        .count = 1,
    },
    [1] = {
        .round = 2,
        .size  = ROUND2_DATA_SIZE,
        .type  = 0x77,
        .shuffle = 0,
        .count = 1,
    },
    [2] = {
        .round = 3,
        .size  = ROUND3_DATA_SIZE,
        .type  = 0x3A,
        .shuffle = 0x4B,
        .count = 1,
    },
    [3] = {
        .round = 4,
        .size  = ROUND4_DATA_SIZE,
        .type  = 0x4F,
        .shuffle = 0x37,
        .count = 1,
    },
    [4] = {
        .round = 5,
        .size  = ROUND5_DATA_SIZE,
        .type  = 0x5E,
        .shuffle = 0xC7,
        .count = 1,
    },
    [5] = {
        .round = 6,
        .size  = ROUND6_DATA_SIZE,
        .type  = 0x6D,
        .shuffle = 0xB2,
        .count = 1,
    },
    [6] = {
        .round = 7,
        .size  = ROUND7_DATA_SIZE,
        .type  = 0x76,
        .shuffle = 0x3B,
        .count = 1,
    },
    [7] = {
        .round = 8,
        .size  = ROUND8_DATA_SIZE,
        .type  = 0x80,
        .shuffle = 0x97,
        .count = 1,
    },
    [8] = {
        .round = 9,
        .size  = ROUND9_DATA_SIZE,
        .type  = 0x93,
        .shuffle = 0x04,
        .count = 1,
    },
    [9] = {
        .round = 10,
        .size  = ROUND10_DATA_SIZE,
        .type  = 0xAA,
        .shuffle = 0x21,
        .count = 1,
    },
    [10] = {
        .round = 11,
        .size  = ROUND11_DATA_SIZE,
        .type  = 0xB2,
        .shuffle = 0x3F,
        .count = 1,
    },
    [11] = {
        .round = 12,
        .size  = ROUND12_DATA_SIZE,
        .type  = 0xCC,
        .shuffle = 0x6F,
        .count = 1,
    },
};

int32_t ClientTesterImpl::testCustomDataRound1(int32_t *count)
{
    int32_t rc = NO_ERROR;
    uint8_t data[ROUND1_DATA_SIZE];

    if (SUCCEED(rc)) {
        data[0] = 0xDE;
        *count = 1;
        rc = mSirius.sendData(gTestInfo[0].type,
            (void *)data, gTestInfo[0].size);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to send data, round %d size %d",
                gTestInfo[0].round, gTestInfo[0].size);
        }
    }

    return rc;
}

int32_t ServerTesterImpl::cbDataRound1(int32_t type, void *_data, int32_t size)
{
    int32_t rc = NO_ERROR;

    if (type != gTestInfo[0].type) {
        LOGE(mModule, "Invalid data type 0x%d", type);
        rc = TEST_FAILED;
    }

    if (size != gTestInfo[0].size) {
        LOGE(mModule, "Invalid data size received %d/%d",
            size, ROUND1_DATA_SIZE);
        rc = TEST_FAILED;
    }

    if (SUCCEED(rc)) {
        uint8_t *data = (uint8_t *)_data;
        if (data[0] != 0xDE) {
            LOGE(mModule, "Failed to verify data in round %d", gTestInfo[0].round);
            rc = TEST_FAILED;
        }
    }

    return rc;
}

int32_t ClientTesterImpl::testCustomDataRound2(int32_t *count)
{
    int32_t rc = NO_ERROR;
    uint8_t data[ROUND2_DATA_SIZE];

    if (SUCCEED(rc)) {
        data[0] = 0xBE;
        data[1] = 0xEF;
        *count = 1;
        rc = mSirius.sendData(gTestInfo[1].type,
            (void *)data, gTestInfo[1].size);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to send data, round %d size %d",
                gTestInfo[1].round, gTestInfo[1].size);
        }
    }

    return rc;
}

int32_t ServerTesterImpl::cbDataRound2(int32_t type, void *_data, int32_t size)
{
    int32_t rc = NO_ERROR;

    if (type != gTestInfo[1].type) {
        LOGE(mModule, "Invalid data type 0x%d", type);
        rc = TEST_FAILED;
    }

    if (size != gTestInfo[1].size) {
        LOGE(mModule, "Invalid data size received %d/%d",
            size, gTestInfo[1].size);
        rc = TEST_FAILED;
    }

    if (SUCCEED(rc)) {
        uint8_t *data = (uint8_t *)_data;
        if (data[0] != 0xBE || data[1] != 0xEF) {
            LOGE(mModule, "Failed to verify data in round %d", gTestInfo[1].round);
            rc = TEST_FAILED;
        }
    }

    return rc;
}

// ----------------------------------------------------------------------------------------

int32_t ClientTesterImpl::testCustomDataRound(int32_t round, uint8_t *data)
{
    int32_t rc = NO_ERROR;
    int32_t final = rc;
    int32_t r = round - 1;

    if (SUCCEED(rc)) {
        if (ISNULL(data)) {
            data = (uint8_t *)Malloc(gTestInfo[r].size);
            if (ISNULL(data)) {
                LOGE(mModule, "Failed to alloc %d memory",
                    gTestInfo[r].size);
            }
        }
    }

    if (SUCCEED(rc)) {
        for (int32_t i = 0; i < gTestInfo[r].count; i++) {
            for (int32_t j = 0; j < gTestInfo[r].size; j++) {
                data[j] = (gTestInfo[r].shuffle + 2 * i + j) % 0xFF;
            }

            rc = mSirius.sendData(gTestInfo[r].type,
                (void *)data, gTestInfo[r].size);
            if (!SUCCEED(rc)) {
                LOGE(mModule, "Failed to send data, round %d size %d"
                    " type %d count %d", round, gTestInfo[r].size,
                    gTestInfo[r].type, gTestInfo[r].count);
                final |= rc;
            }
        }
    }

    SECURE_FREE(data);

    return rc | final;
}

int32_t ServerTesterImpl::cbDataRound(
    int32_t round, int32_t type, void *_data, int32_t size)
{
    int32_t rc = NO_ERROR;
    int32_t r = round - 1;

    if (type != gTestInfo[r].type) {
        LOGE(mModule, "Invalid data type %d/%d", type, gTestInfo[r].type);
        rc = TEST_FAILED;
    }

    if (size != gTestInfo[r].size) {
        LOGE(mModule, "Invalid data size received %d/%d",
            size, gTestInfo[r].size);
        rc = TEST_FAILED;
    }

    if (SUCCEED(rc)) {
        uint8_t *data = (uint8_t *)_data;
        int8_t diff = data[1] - data[0];
        diff = diff > 0 ? diff : diff + 0xFF;
        for (int32_t i = 1; i < size; i++) {
            if ((data[i - 1] + diff) % 0xFF != data[i]) {
                LOGE(mModule, "Failed to verify data in "
                    "round %d data[i-1] %d data[i] %d diff %d",
                    round, data[i - 1], data[i], diff);
                rc = TEST_FAILED;
                break;
            }
        }
    }

    if (SUCCEED(rc)){
        LOGD(mModule, "Receievd custom data on server and checked alright."
            "round %d type %d size %d <-", round, type, size);
    } else {
        LOGE(mModule, "Received custom data but error detected."
            "round %d type %d size %d <-", round, type, size);
    }

    return rc;
}

int32_t ClientTesterImpl::testCustomDataRound3(int32_t *count)
{
    *count = 1;
    return testCustomDataRound(3, NULL);
}

int32_t ServerTesterImpl::cbDataRound3(int32_t type, void *data, int32_t size)
{
    return cbDataRound(3, type, data, size);
}

int32_t ClientTesterImpl::testCustomDataRound4(int32_t *count)
{
    *count = 1;
    return testCustomDataRound(4, NULL);
}

int32_t ServerTesterImpl::cbDataRound4(int32_t type, void *data, int32_t size)
{
    return cbDataRound(4, type, data, size);
}

int32_t ClientTesterImpl::testCustomDataRound5(int32_t *count)
{
    *count = 1;
    return testCustomDataRound(5, NULL);
}

int32_t ServerTesterImpl::cbDataRound5(int32_t type, void *data, int32_t size)
{
    return cbDataRound(5, type, data, size);
}

int32_t ClientTesterImpl::testCustomDataRound6(int32_t *count)
{
    *count = 1;
    return testCustomDataRound(6, NULL);
}

int32_t ServerTesterImpl::cbDataRound6(int32_t type, void *data, int32_t size)
{
    return cbDataRound(6, type, data, size);
}

int32_t ClientTesterImpl::testCustomDataRound7(int32_t *count)
{
    *count = 1;
    return testCustomDataRound(7, NULL);
}

int32_t ServerTesterImpl::cbDataRound7(int32_t type, void *data, int32_t size)
{
    return cbDataRound(7, type, data, size);
}

int32_t ClientTesterImpl::testCustomDataRound8(int32_t *count)
{
    *count = 1;
    return testCustomDataRound(8, NULL);
}

int32_t ServerTesterImpl::cbDataRound8(int32_t type, void *data, int32_t size)
{
    return cbDataRound(8, type, data, size);
}

int32_t ClientTesterImpl::testCustomDataRound9(int32_t *count)
{
    *count = 1;
    return testCustomDataRound(9, NULL);
}

int32_t ServerTesterImpl::cbDataRound9(int32_t type, void *data, int32_t size)
{
    return cbDataRound(9, type, data, size);
}

int32_t ClientTesterImpl::testCustomDataRound10(int32_t *count)
{
    *count = 1;
    return testCustomDataRound(10, NULL);
}

int32_t ServerTesterImpl::cbDataRound10(int32_t type, void *data, int32_t size)
{
    return cbDataRound(10, type, data, size);
}

int32_t ClientTesterImpl::testCustomDataRound11(int32_t *count)
{
    *count = 1;
    return testCustomDataRound(11, NULL);
}

int32_t ServerTesterImpl::cbDataRound11(int32_t type, void *data, int32_t size)
{
    return cbDataRound(11, type, data, size);
}

int32_t ClientTesterImpl::testCustomDataRound12(int32_t *count)
{
    *count = 1;
    return testCustomDataRound(12, NULL);
}

int32_t ServerTesterImpl::cbDataRound12(int32_t type, void *data, int32_t size)
{
    return cbDataRound(12, type, data, size);
}

// ----------------------------------------------------------------------------------------

const ClientTesterImpl::TestCaseFunction ClientTesterImpl::gCustomDataSingleTestCases[] = {
    [0] = &ClientTesterImpl::testCustomDataRound1,
    [1] = &ClientTesterImpl::testCustomDataRound2,
    [2] = &ClientTesterImpl::testCustomDataRound3,
    [3] = &ClientTesterImpl::testCustomDataRound4,
    [4] = &ClientTesterImpl::testCustomDataRound5,
    [5] = &ClientTesterImpl::testCustomDataRound6,
    [6] = &ClientTesterImpl::testCustomDataRound7,
    [7] = &ClientTesterImpl::testCustomDataRound8,
    [8] = &ClientTesterImpl::testCustomDataRound9,
    [9] = &ClientTesterImpl::testCustomDataRound10,
    [10] = &ClientTesterImpl::testCustomDataRound11,
    [11] = &ClientTesterImpl::testCustomDataRound12,
};

const uint32_t ClientTesterImpl::gCustomDataSingleTestCaseSize =
    ARRAYSIZE(ClientTesterImpl::gCustomDataSingleTestCases);

int32_t ClientTesterImpl::testCustomDataSingleCases(int32_t *count, int32_t ms)
{
    int32_t rc = NO_ERROR;
    int32_t final = rc;

    for (uint32_t i = 0; i < ARRAYSIZE(gCustomDataSingleTestCases); i++) {
        int32_t tmp = 0;
        rc = (this->*(gCustomDataSingleTestCases[i]))(&tmp);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to run %s test case, round %d, %d",
                getRequestName(CUSTOM_DATA), i + 1, rc);
            final |= rc;
        }

        delayMs(ms);
    }

    *count = ARRAYSIZE(gCustomDataSingleTestCases);

    return final;
}

int32_t ClientTesterImpl::testCustomDataSingleCasesReverse(
    int32_t *count, int32_t ms)
{
    int32_t rc = NO_ERROR;
    int32_t final = rc;

    for (int32_t i = ARRAYSIZE(gCustomDataSingleTestCases) - 1; i >= 0; i--) {
        int32_t tmp = 0;
        rc = (this->*(gCustomDataSingleTestCases[i]))(&tmp);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to run %s test case, round %d, %d",
                getRequestName(CUSTOM_DATA), i + 1, rc);
            final |= rc;
        }

        delayMs(ms);
    }

    *count = ARRAYSIZE(gCustomDataSingleTestCases);

    return final;
}

int32_t ClientTesterImpl::testCustomDataRoundOneThread(
    int32_t *count, int32_t round, int32_t delayMs)
{
    int32_t rc = NO_ERROR;
    int32_t final = rc;
    int32_t tmp = 0;

    for (int32_t i = 0; i < round; i++) {
        rc = i % 2 ?
            testCustomDataSingleCases(&tmp, delayMs) :
            testCustomDataSingleCasesReverse(&tmp, delayMs);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to run all single cases, %d", rc);
            final |= rc;
        }
    }

    *count = tmp * round;

    return final;
}

int32_t ServerTesterImpl::cbDataRoundOneThread(
    int32_t type, void *data, int32_t size)
{
    int32_t rc = NOT_FOUND;

    for (uint32_t i = 0; i < ARRAYSIZE(gTestInfo); i++) {
        if (type == gTestInfo[i].type) {
            rc = cbDataRound(gTestInfo[i].round, type, data, size);
            if (!SUCCEED(rc)) {
                LOGE(mModule, "Received mistaken data, %d", rc);
            } else {
                rc = NO_ERROR;
            }
            break;
        }
    }

    return rc;
}

int32_t ClientTesterImpl::testCustomDataRound13(int32_t *count)
{
    return testCustomDataRoundOneThread(count,
        ROUND13_ONETHREAD_ROUND, ROUND13_ROUND_DELAY);
}

int32_t ServerTesterImpl::cbDataRound13(int32_t type, void *data, int32_t size)
{
    return cbDataRoundOneThread(type, data, size);
}

int32_t ClientTesterImpl::testCustomDataRound14(int32_t *count)
{
    return testCustomDataRoundOneThread(count,
        ROUND14_ONETHREAD_ROUND, ROUND14_ROUND_DELAY);
}

int32_t ServerTesterImpl::cbDataRound14(int32_t type, void *data, int32_t size)
{
    return cbDataRoundOneThread(type, data, size);
}

// ----------------------------------------------------------------------------------------

int32_t ClientTesterImpl::testCustomDataRoundMultiThread(
    int32_t *count, int32_t round, int32_t delayRound, int32_t delayCase)
{
    int32_t rc = NO_ERROR;
    int32_t final = rc;
    Semaphore sem;

    for (int32_t i = 0; i < round; i++) {
        rc = mThreads->run(
            [&]() -> int32_t {
                int32_t tmp = 0;
                int32_t _rc = i % 2 ?
                    testCustomDataSingleCases(&tmp, delayCase) :
                    testCustomDataSingleCasesReverse(&tmp, delayCase);
                sem.signal();
                if (!SUCCEED(_rc)) {
                    LOGE(mModule, "Failed to run all single cases, %d", _rc);
                    final |= _rc;
                }
                return _rc;
            }
        );
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to run on thread, %d", rc);
        }

        delayMs(delayRound);
        RESETRESULT(rc);
    }

    if (SUCCEED(rc)) {
        for (int32_t i = 0; i < round; i++) {
            // Wait all threads to finish task
            sem.wait();
        }
    }

    if (SUCCEED(rc) || !SUCCEED(rc)) {
        *count = ARRAYSIZE(gCustomDataSingleTestCases) * round;
    }

    return final;
}

int32_t ClientTesterImpl::testCustomDataRound15(int32_t *count)
{
    return testCustomDataRoundMultiThread(count,
        ROUND15_MULTITHREAD_NUM, ROUND15_LOOP_DELAY, ROUND15_CASE_DELAY);
}

int32_t ServerTesterImpl::cbDataRound15(int32_t type, void *data, int32_t size)
{
    return cbDataRoundOneThread(type, data, size);
}

int32_t ClientTesterImpl::testCustomDataRound16(int32_t *count)
{
    return testCustomDataRoundMultiThread(count,
        ROUND16_MULTITHREAD_NUM, ROUND16_LOOP_DELAY, ROUND16_CASE_DELAY);
}

int32_t ServerTesterImpl::cbDataRound16(int32_t type, void *data, int32_t size)
{
    return cbDataRoundOneThread(type, data, size);
}

int32_t ServerTesterImpl::findMatchTypeSize(int32_t type, int32_t size)
{
    int32_t rc = NOT_FOUND;

    for (uint32_t i = 0; i < ARRAYSIZE(gTestInfo); i++) {
        if (gTestInfo[i].type == type &&
            gTestInfo[i].size == size) {
            rc = NO_ERROR;
            break;
        }
    }

    return rc;
}

};

