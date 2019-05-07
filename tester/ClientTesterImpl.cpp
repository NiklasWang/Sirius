#include "common.h"
#include "configuration.h"
#include "ClientTesterImpl.h"
#include "YuvPictureOperator.h"

namespace tester {

using namespace sirius;

int32_t ClientTesterImpl::construct()
{
    int32_t rc = NO_ERROR;

    if (mConstructed) {
        rc = ALREADY_INITED;
    }

    if (SUCCEED(rc)) {
        if (ISNULL(mThreads)) {
            mThreads = ThreadPoolEx::getInstance();
            if (ISNULL(mThreads)) {
                LOGE(mModule, "Failed to get thread pool");
                rc = NOT_READY;
            }
        }
    }

    if (SUCCEED(rc)) {
        rc = mSC.construct();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to construct server state machine, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = mBackupSC.construct();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to construct server state machine, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        mConstructed = true;
        LOGD(mModule, "Client tester constructed");
    }

    return rc;
}

int32_t ClientTesterImpl::destruct()
{
    int32_t rc = NO_ERROR;
    int32_t final = NO_ERROR;

    if (!mConstructed) {
        rc = NOT_INITED;
    } else {
        mConstructed = false;
    }

    if (SUCCEED(rc)) {
        rc = mSC.destruct();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to destruct client socket SM, %d", rc);
            final |= rc;
            RESETRESULT(rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = mBackupSC.destruct();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to destruct client socket SM, %d", rc);
            final |= rc;
            RESETRESULT(rc);
        }
    }

    if (SUCCEED(rc)) {
        if (NOTNULL(mThreads)) {
            mThreads->removeInstance();
            mThreads = NULL;
        }
    }

    if (!SUCCEED(final)) {
        LOGE(mModule, "Client tester destructed with error %d", final);
    } else {
        LOGD(mModule, "Client tester destructed");
    }

    return rc;
}

int32_t ClientTesterImpl::prepare()
{
    int32_t rc = NO_ERROR;
    Header header;

    if (SUCCEED(rc)) {
        if (!mConstructed) {
            rc = construct();
            if (!SUCCEED(rc)) {
                LOGE(mModule, "Failed to construct client tester, %d", rc);
            }
        }
    }

    if (SUCCEED(rc)) {
        rc = mSC.connectServer();
        if (rc == NOT_READY) {
            LOGI(mModule, "Server may not started, exit...");
        } else if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to connect to server, %d", rc);
        }
    }


    do {
        rc = mBackupSC.connectServer();
        if (rc == NOT_READY) {
            LOGI(mModule, "Server may not started, retry in 1 seconds...");
            delaySec(1);
        } else if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to connect to server, %d", rc);
        }
    } while (rc == NOT_READY);

    if (SUCCEED(rc)) {
        int32_t w, h, stride, scanline;
        rc = getLogoSize(0, &w, &h, &stride, &scanline);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to get logo size, %d", rc);
        } else {
            header.previewW = w;
            header.previewH = h;
            header.previewStride   = stride;
            header.previewScanline = scanline;
        }
    }

    if (SUCCEED(rc)) {
        int32_t w, h, stride, scanline;
        rc = getLogoSize(1, &w, &h, &stride, &scanline);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to get logo size, %d", rc);
        } else {
            header.pictureW = w;
            header.pictureH = h;
            header.pictureStride   = stride;
            header.pictureScanline = scanline;
        }
    }

    if (SUCCEED(rc)) {
        int32_t w, h, stride, scanline;
        rc = getLogoSize(2, &w, &h, &stride, &scanline);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to get logo size, %d", rc);
        } else {
            header.bayerPictureW = w;
            header.bayerPictureH = h;
            header.bayerPattern  = BAYER_PATTERN_RGGB;
        }
    }

    if (SUCCEED(rc)) {
        rc = mSirius.init(header);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to init header, %d", rc);
        }
    }

    return rc;
}

ClientTesterImpl::ClientTesterImpl() :
    mModule(MODULE_CLIENT_TESTER),
    mConstructed(false),
    mThreads(NULL),
    mSC(TESTER_SOCKET_NAME),
    mBackupSC(TESTER_SOCKET_NAME)
{
}

ClientTesterImpl::~ClientTesterImpl()
{
    if (mConstructed) {
        destruct();
    }
}

const ClientTesterImpl::TestCaseFunction ClientTesterImpl::gPreviewTestCases[] = {
    [0] = &ClientTesterImpl::testPreviewRound1,
    [1] = &ClientTesterImpl::testPreviewRound2,
    [2] = &ClientTesterImpl::testPreviewRound3,
    [3] = &ClientTesterImpl::testPreviewRound4,
    [4] = &ClientTesterImpl::testPreviewRound5,
    [5] = &ClientTesterImpl::testPreviewRound6,
    [6] = &ClientTesterImpl::testPreviewRound7,
};

int32_t ClientTesterImpl::testPreview(int32_t round)
{
    int32_t rc = NO_ERROR;
    int32_t count = 0;

    if (SUCCEED(rc)) {
        rc = SyncWithServerToStart(PREVIEW_NV21, round);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to start test, sync server %s round %d, %d",
                getRequestName(PREVIEW_NV21), round, rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = (this->*(gPreviewTestCases[round]))(&count);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to run client test case, %s round %d, %d",
                getRequestName(PREVIEW_NV21), round, rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = SyncWithServerToFinish(PREVIEW_NV21, round, count);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to sync with server to finish case %d, "
                "round %d, %d", getRequestName(PREVIEW_NV21), round, rc);
        }
    }

    return rc;
}

const ClientTesterImpl::TestCaseFunction ClientTesterImpl::gNV21PictureTestCases[] = {
    [0] = &ClientTesterImpl::testNV21PictureRound1,
    [1] = &ClientTesterImpl::testNV21PictureRound2,
    [2] = &ClientTesterImpl::testNV21PictureRound3,
    [3] = &ClientTesterImpl::testNV21PictureRound4,
    [4] = &ClientTesterImpl::testNV21PictureRound5,
    [5] = &ClientTesterImpl::testNV21PictureRound6,
    [6] = &ClientTesterImpl::testNV21PictureRound7,
};

int32_t ClientTesterImpl::testNV21Picture(int32_t round)
{
    int32_t rc = NO_ERROR;
    int32_t count = 0;

    if (SUCCEED(rc)) {
        rc = SyncWithServerToStart(PICTURE_NV21, round);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to start test, sync server %s round %d, %d",
                getRequestName(PICTURE_NV21), round, rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = (this->*(gNV21PictureTestCases[round]))(&count);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to run client test case, %s round %d, %d",
                getRequestName(PICTURE_NV21), round, rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = SyncWithServerToFinish(PICTURE_NV21, round, count);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to sync with server to finish case %d, "
                "round %d, %d", getRequestName(PICTURE_NV21), round, rc);
        }
    }

    return rc;
}

const ClientTesterImpl::TestCaseFunction ClientTesterImpl::gBayerPictureTestCases[] = {
    [0] = &ClientTesterImpl::testBayerPictureRound1,
    [1] = &ClientTesterImpl::testBayerPictureRound2,
    [2] = &ClientTesterImpl::testBayerPictureRound3,
    [3] = &ClientTesterImpl::testBayerPictureRound4,
    [4] = &ClientTesterImpl::testBayerPictureRound5,
    [5] = &ClientTesterImpl::testBayerPictureRound6,
    [6] = &ClientTesterImpl::testBayerPictureRound7,
};

int32_t ClientTesterImpl::testBayerPicture(int32_t round)
{
    int32_t rc = NO_ERROR;
    int32_t count = 0;

    if (SUCCEED(rc)) {
        rc = SyncWithServerToStart(PICTURE_BAYER, round);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to start test, sync server %s round %d, %d",
                getRequestName(PICTURE_BAYER), round, rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = (this->*(gBayerPictureTestCases[round]))(&count);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to run client test case, %s round %d, %d",
                getRequestName(PICTURE_BAYER), round, rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = SyncWithServerToFinish(PICTURE_BAYER, round, count);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to sync with server to finish case %d, "
                "round %d, %d", getRequestName(PICTURE_BAYER), round, rc);
        }
    }

    return rc;
}

const ClientTesterImpl::TestCaseFunction ClientTesterImpl::gEventTestCases[] = {
    [0] = &ClientTesterImpl::testEventsRound1,
    [1] = &ClientTesterImpl::testEventsRound2,
    [2] = &ClientTesterImpl::testEventsRound3,
    [3] = &ClientTesterImpl::testEventsRound4,
    [4] = &ClientTesterImpl::testEventsRound5,
    [5] = &ClientTesterImpl::testEventsRound6,
    [6] = &ClientTesterImpl::testEventsRound7,
};

int32_t ClientTesterImpl::testEvents(int32_t round)
{
    int32_t rc = NO_ERROR;
    int32_t count = 0;

    if (SUCCEED(rc)) {
        rc = SyncWithServerToStart(EXTENDED_EVENT, round);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to start test, sync server %s round %d, %d",
                getRequestName(EXTENDED_EVENT), round, rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = (this->*(gEventTestCases[round]))(&count);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to run client test case, %s round %d, %d",
                getRequestName(EXTENDED_EVENT), round, rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = SyncWithServerToFinish(EXTENDED_EVENT, round, count);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to sync with server to finish case %d, "
                "round %d, %d", getRequestName(EXTENDED_EVENT), round, rc);
        }
    }

    return rc;
}

const ClientTesterImpl::TestCaseFunction ClientTesterImpl::gCustomDataTestCases[] = {
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
    [12] = &ClientTesterImpl::testCustomDataRound13,
    [13] = &ClientTesterImpl::testCustomDataRound14,
    [14] = &ClientTesterImpl::testCustomDataRound15,
};

int32_t ClientTesterImpl::testCustomData(int32_t round)
{
    int32_t rc = NO_ERROR;
    int32_t count = 0;

    if (SUCCEED(rc)) {
        rc = SyncWithServerToStart(CUSTOM_DATA, round);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to start test, sync server %s round %d, %d",
                getRequestName(CUSTOM_DATA), round, rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = (this->*(gCustomDataTestCases[round]))(&count);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to run client test case, %s round %d, %d",
                getRequestName(CUSTOM_DATA), round, rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = SyncWithServerToFinish(CUSTOM_DATA, round, count);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to sync with server to finish case %d, "
                "round %d, %d", getRequestName(CUSTOM_DATA), round, rc);
        }
    }

    return rc;
}

const ClientTesterImpl::TestCaseFunction ClientTesterImpl::gMixtureTestCases[] = {
    [0] = &ClientTesterImpl::testMixtureRound1,
    [1] = &ClientTesterImpl::testMixtureRound2,
    [2] = &ClientTesterImpl::testMixtureRound3,
};

int32_t ClientTesterImpl::testMixture(int32_t round)
{
    int32_t rc = NO_ERROR;
    int32_t count = 0;

    if (SUCCEED(rc)) {
        rc = SyncWithServerToStart(REQUEST_TYPE_MAX_INVALID, round);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to start test, sync server "
                "%s round %d, %d", getRequestName(
                    REQUEST_TYPE_MAX_INVALID), round, rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = (this->*(gMixtureTestCases[round]))(&count);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to run client test case, "
                "%s round %d, %d", getRequestName(
                    REQUEST_TYPE_MAX_INVALID), round, rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = SyncWithServerToFinish(REQUEST_TYPE_MAX_INVALID, round, count);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to sync with server to finish case %d, "
                "round %d, %d", getRequestName(
                    REQUEST_TYPE_MAX_INVALID), round, rc);
        }
    }

    return rc;
}

int32_t ClientTesterImpl::SyncWithServerToStart(RequestType type, int32_t round)
{
    int32_t rc = NO_ERROR;
    char msg[SOCKET_DATA_MAX_LEN];
    RWLock::AutoWLock l(mSocketLocker);

    if (SUCCEED(rc)) {
        sprintf(msg, TESTER_START_TEST " %d %d", type, round);
        rc = mSC.sendMsg(msg, strlen(msg));
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to send msg %s, %d", msg, rc);
        }
    }

    if (SUCCEED(rc)) {
        msg[0] = '\0';
        rc = mSC.receiveMsg(msg, SOCKET_DATA_MAX_LEN);
        msg[SOCKET_DATA_MAX_LEN - 1] = '\0';
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to receive server msg, %d", msg);
        }
    }

    if (SUCCEED(rc)) {
        if (!COMPARE_SAME_LEN_STRING(msg, TESTER_SERVER_REPLY_OK,
            strlen(TESTER_SERVER_REPLY_OK))) {
            LOGE(mModule, "Server test not ok, msg %s received.", msg, rc);
            rc = TEST_FAILED;
        }
    }

    return rc;
}

int32_t ClientTesterImpl::SyncWithServerToFinish(RequestType type, int32_t round, int32_t count)
{
    int32_t rc = NO_ERROR;
    char msg[SOCKET_DATA_MAX_LEN];
    RWLock::AutoWLock l(mSocketLocker);

    if (SUCCEED(rc)) {
        sprintf(msg, TESTER_REPORT_STATUS " %d %d %d", type, round, count);
        rc = mBackupSC.sendMsg(msg, strlen(msg));
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to send msg %s, %d", msg, rc);
        }
    }

    if (SUCCEED(rc)) {
        msg[0] = '\0';
        rc = mBackupSC.receiveMsg(msg, SOCKET_DATA_MAX_LEN);
        msg[SOCKET_DATA_MAX_LEN - 1] = '\0';
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to receive server msg, %d", msg);
        }
    }

    if (SUCCEED(rc)) {
        bool succeed = false;
        rc = translateServerMsgToStatus(msg, TESTER_SERVER_FINISH, &succeed);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to get server test status, %d", rc);
        } else {
            if (succeed) {
                LOGI(mModule, "Server test on %s round %d succeed.",
                    getRequestName(type), round);
            } else {
                LOGE(mModule, "Server test on %s round %d FAILED! "
                    "Check log in server side.",
                    getRequestName(type), round);
                rc = TEST_FAILED;
            }
        }
    }

    mSirius.abort(type);

    return rc;
}

int32_t ClientTesterImpl::syncServerExit()
{
    RWLock::AutoWLock l(mSocketLocker);
    int32_t rc = mSC.sendMsg(TESTER_CLIENT_EXIT,
        strlen(TESTER_CLIENT_EXIT));
    if (!SUCCEED(rc)) {
        LOGE(mModule, "Failed to send server "
            "exitmsg, %d", rc);
    }

    return rc;
}

int32_t ClientTesterImpl::translateServerMsgToStatus(
    char *msg, const char *prefix, bool *succeed)
{
    int32_t rc = NO_ERROR;
    int32_t result = 0;

    if (!COMPARE_SAME_LEN_STRING(msg, prefix, strlen(prefix))) {
        LOGE(mModule, "Prefix not match, %s VS %s", msg, prefix);
        rc = PARAM_INVALID;
    }

    if (SUCCEED(rc)) {
        result = atoi(msg + strlen(prefix) + 1);
        if (result < 0 || result > 1) {
            LOGE(mModule, "Invalid msg, \"%s\"", msg);
            rc = INVALID_FORMAT;
        } else {
            *succeed = result == 1;
        }
    }

    return rc;
}

};

