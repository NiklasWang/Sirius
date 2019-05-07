#include "common.h"
#include "SiriusServer.h"
#include "ServerTesterImpl.h"

#define PREVIEW_WAIT_MAX_TIME       500  // ms
#define NV21_PICTURE_WAIT_MAX_TIME  1000
#define BAYER_PICTURE_WAIT_MAX_TIME 1000
#define EVENT_WAIT_MAX_TIME         300
#define CUSTOM_DATA_MAX_WAIT_TIME   2000
#define MIXTURE_MAX_WAIT_TIME       4000

namespace tester {

using namespace sirius;

int32_t ServerTesterImpl::start()
{
    int32_t rc = NO_ERROR;
    int32_t mbackupfd = -1;

    if (SUCCEED(rc)) {
        if (!mConstructed) {
            rc = construct();
            if (!SUCCEED(rc)) {
                LOGE(mModule, "Failed to construct server tester, %d" ,rc);
            }
        }
    }

    if (SUCCEED(rc)) {
        rc = resetSirius();
        if (SUCCEED(rc)) {
            LOGE(mModule, "Failed to reset sirius server, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = mSS.startServer();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to start server, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = mSS.waitForConnect();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to wait for connection, %d", rc);
        } else {
            LOGI(mModule, "Tester client connected.");
        }
    }

    if (SUCCEED(rc)) {
        rc = mSS.waitForConnect(&mbackupfd);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to wait for connection, %d", rc);
        } else {
            LOGI(mModule, "Tester client connected.");
        }
    }

    if (SUCCEED(rc)) {
        rc = mBackupSS.setClientFd(mbackupfd);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to wait for connection, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = runTests();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Run server test failed, %d", rc);
        }
    }

    return rc;
}

int32_t ServerTesterImpl::runTests()
{
    int32_t rc = NO_ERROR;

    do {
        rc = runTest();
        if (rc == JUMP_DONE) {
            LOGE(mModule, "Server tester will exit.");
        } else if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to run test, %d", rc);
        }
    } while (rc != JUMP_DONE);

    return RETURNIGNORE(rc, JUMP_DONE);
}

int32_t ServerTesterImpl::runTest()
{
    int32_t rc = NO_ERROR;
    char msg[SOCKET_DATA_MAX_LEN];
    RequestType request = REQUEST_TYPE_MAX_INVALID;
    int32_t round = -1;

    if (SUCCEED(rc)) {
        msg[0] = '\0';
        rc = mSS.receiveMsg(msg, SOCKET_DATA_MAX_LEN);
        msg[SOCKET_DATA_MAX_LEN - 1] = '\0';
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to receive server msg, %d", msg);
            rc = JUMP_DONE;
        }
    }

    if (SUCCEED(rc)) {
        if (COMPARE_SAME_LEN_STRING(msg, TESTER_CLIENT_EXIT,
            strlen(TESTER_CLIENT_EXIT))) {
            LOGI(mModule, "Client tester exit notification received.");
            rc = JUMP_DONE;
        }
    }

    if (SUCCEED(rc)) {
        if (!COMPARE_SAME_LEN_STRING(msg, TESTER_START_TEST,
            strlen(TESTER_START_TEST))) {
            LOGE(mModule, "Invalid msg %s received.", msg, rc);
            rc = BAD_PROTOCAL;
        }
    }

    if (SUCCEED(rc)) {
        rc = convertMsgToRequest(msg, TESTER_START_TEST,
            &request, &round);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to convert msg to "
                "request, %s", msg);
        }
    }

    if (SUCCEED(rc)) {
        if (request >= 0 && request <= REQUEST_TYPE_MAX_INVALID) {
            mTestingRequest = request;
            mTestingRound = round;
        } else {
            LOGE(mModule, "Not supported request %d round %d",
                request, round);
            rc = BAD_PROTOCAL;
        }
    }

    if (SUCCEED(rc)) {
        if (mTestingRequest != REQUEST_TYPE_MAX_INVALID) {
            rc = mSirius.request(mTestingRequest);
            if (!SUCCEED(rc)) {
                LOGE(mModule, "Failed to request %s, %d",
                        getRequestName(mTestingRequest), rc);
            }
        } else {
            for (int32_t i = 0; i < REQUEST_TYPE_MAX_INVALID; i++) {
                rc = mSirius.request((RequestType)i);
                if (!SUCCEED(rc)) {
                    LOGE(mModule, "Failed to request %s %d",
                        getRequestName((RequestType)i), rc);
                }
            }
        }
    }

    if (SUCCEED(rc)) {
        mPreviewCnt      = 0;
        mPreviewResult   = true;
        mPreviewIdentity = 0;
        mNV21PictureCnt  = 0;
        mNV21PictureResult    = true;
        mNV21PictureIdentity  = 0;
        mBayerPictureCnt      = 0;
        mBayerPictureResult   = true;
        mBayerPictureIdentity = 0;
        mEventCnt    = 0;
        mEventResult = true;
        mLastEvent = -1;
        mDataCnt     = 0;
        mDataResult  = true;
        mMixtureCnt  = 0;
        mMixtureResult = true;
    }

    if (SUCCEED(rc)) {
        rc = mThreads->run(
            [this]() -> int32_t {
                return startWaittingCb();
            }
        );
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to run waitting loop, %d", rc);
        }
    }

    if (rc != JUMP_DONE) {
        const char *reply = SUCCEED(rc) ?
            TESTER_SERVER_REPLY_OK : TESTER_SERVER_REPLY_FAIL;
        rc = mSS.sendMsg(reply, strlen(reply));
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to reply client msg %s", reply);
        }
    }

    return rc;
}

int32_t ServerTesterImpl::convertMsgToRequest(char *msg,
    const char *prefix, RequestType *request, int32_t *round)
{
    int32_t rc = NO_ERROR;

    if (SUCCEED(rc)) {
        int32_t len = strlen(prefix);
        if (!COMPARE_SAME_LEN_STRING(msg, prefix, len)) {
            LOGE(mModule, "Invalid client msg, %s", msg);
            rc = PARAM_INVALID;
        }
    }

    if (SUCCEED(rc)) {
        int32_t count = sscanf(msg,
            TESTER_START_TEST " %d %d", request, round);
        if (count != 2) {
            LOGE(mModule, "Failed to convert request info "
                "from msg, %s", msg);
            rc = INVALID_FORMAT;
        }
    }

    return rc;
}

int32_t ServerTesterImpl::startWaittingCb()
{
    int32_t rc = NO_ERROR;
    SemaphoreTimeout *sem = NULL;
    int32_t *count = NULL;
    bool *result = NULL;
    char  msg[SOCKET_DATA_MAX_LEN];
    RequestType clientRequest = REQUEST_TYPE_MAX_INVALID;
    int32_t clientRound = -1;
    int32_t clientCount = -1;

    if (SUCCEED(rc)) {
        switch (mTestingRequest) {
            case PREVIEW_NV21: {
                sem = &mPreviewSem;
                count  = &mPreviewCnt;
                result = &mPreviewResult;
            }; break;
            case PICTURE_NV21: {
                sem = &mNV21PictureSem;
                count  = &mNV21PictureCnt;
                result = &mNV21PictureResult;
            }; break;
            case PICTURE_BAYER: {
                sem = &mBayerPictureSem;
                count  = &mBayerPictureCnt;
                result = &mBayerPictureResult;
            }; break;
            case EXTENDED_EVENT: {
                sem = &mEventSem;
                count  = &mEventCnt;
                result = &mEventResult;
            }; break;
            case CUSTOM_DATA: {
                sem = &mDataSem;
                count  = &mDataCnt;
                result = &mDataResult;
            }; break;
            case REQUEST_TYPE_MAX_INVALID: {
                sem = &mMixtureSem;
                count  = &mMixtureCnt;
                result = &mMixtureResult;
            }; break;
            default: {
                LOGE(mModule, "Unknown tesing request, %s",
                    getRequestName(mTestingRequest));
                rc = UNKNOWN_ERROR;
            }; break;
        }
    }

    if (SUCCEED(rc)) {
        do {
            rc = sem->wait();
            if (rc == ETIMEDOUT) {
                LOGI(mModule, "Wait %s cb timeout, %d callback received, "
                    "test should finished.", getRequestName(
                        mTestingRequest), *count);
            } else if (!SUCCEED(rc)) {
                LOGE(mModule, "Failed to wait %s, %d",
                    getRequestName(mTestingRequest), rc);
            }
        } while (rc != ETIMEDOUT);
        if (rc == ETIMEDOUT) {
            RESETRESULT(rc);
        }
    }

    if (SUCCEED(rc)) {
        msg[0] = '\0';
        rc = mBackupSS.receiveMsg(msg, SOCKET_DATA_MAX_LEN);
        msg[SOCKET_DATA_MAX_LEN - 1] = '\0';
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to receive client msg, %d", msg);
        }
    }

    if (SUCCEED(rc)) {
        if (!COMPARE_SAME_LEN_STRING(msg, TESTER_REPORT_STATUS,
            strlen(TESTER_REPORT_STATUS))) {
            LOGE(mModule, "Invalid msg %s, unknown to handle.", msg);
            rc = BAD_PROTOCAL;
        }
    }

    if (SUCCEED(rc)) {
        rc = convertMsgToStat(msg, TESTER_REPORT_STATUS,
            &clientRequest, &clientRound, &clientCount);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to convert msg to "
                "test stat info, %s", msg);
        }
    }

    if (SUCCEED(rc)) {
        if (clientRequest != mTestingRequest ||
            clientRound != mTestingRound) {
            LOGE(mModule, "Should NOT be here, test mismatch "
                "Testing request %d/%d, round %d/%d", clientRequest,
                mTestingRequest, clientRound, mTestingRound);
            rc = BAD_PROTOCAL;
        }
    }

    if (SUCCEED(rc)) {
        if (mTestingRequest == EXTENDED_EVENT ||
            mTestingRequest == CUSTOM_DATA) {
            if (*count != clientCount) {
                LOGE(mModule, "Client sent %d %s in round %d but server "
                    "received %d, mismatch. Event or data must NOT lost "
                    "during transmission.", clientCount,
                    getRequestName(mTestingRequest), mTestingRound, *count);
                rc = TEST_FAILED;
            }
        }
    }

    if (SUCCEED(rc)) {
        LOGI(mModule, "Request %s Round %d test result: %s.",
            getRequestName(mTestingRequest), mTestingRound,
            result ? "Succeed" : "FAILED");
        LOGI(mModule, "Request %s, Round %d Client sent %d, server "
            "received %d, Lost Rate %.2f, Received Rate %.2f",
            getRequestName(mTestingRequest), mTestingRound,
            clientCount, *count, (clientCount - *count) * 1.0f / clientCount,
            *count * 1.0f / clientCount);
    }

    if (SUCCEED(rc) || !SUCCEED(rc)) {
        if (mTestingRequest != REQUEST_TYPE_MAX_INVALID) {
            rc = mSirius.abort(mTestingRequest);
            if (!SUCCEED(rc)) {
                LOGE(mModule, "Failed to abort %s, %d",
                    getRequestName(mTestingRequest), rc);
            }
        }
    }

    if (SUCCEED(rc) || !SUCCEED(rc)) {
        if (mTestingRequest == REQUEST_TYPE_MAX_INVALID) {
            for (int32_t i = 0; i < REQUEST_TYPE_MAX_INVALID; i++) {
                rc |= mSirius.abort((RequestType)i);
                if (!SUCCEED(rc)) {
                    LOGE(mModule, "Failed to abort %s %d",
                        getRequestName((RequestType)i), rc);
                }
            }
        }
    }

    if (SUCCEED(rc)) {
        sprintf(msg, TESTER_SERVER_FINISH " %d", *result ? 1 : 0);
        rc = mBackupSS.sendMsg(msg, strlen(msg));
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to send msg %s, %d", msg, rc);
        }
    }

    return rc;
}

int32_t ServerTesterImpl::convertMsgToStat(char *msg,
    const char *prefix, RequestType *request, int32_t *round, int32_t *count)
{
    int32_t rc = NO_ERROR;

    if (SUCCEED(rc)) {
        int32_t len = strlen(prefix);
        if (!COMPARE_SAME_LEN_STRING(msg, prefix, len)) {
            LOGE(mModule, "Invalid client msg, %s", msg);
            rc = PARAM_INVALID;
        }
    }

    if (SUCCEED(rc)) {
        int32_t cnt = sscanf(msg,
            TESTER_REPORT_STATUS " %d %d %d", request, round, count);
        if (cnt != 3) {
            LOGE(mModule, "Failed to convert request info "
                "from msg, %s", msg);
            rc = INVALID_FORMAT;
        }
    }

    return rc;
}

int32_t ServerTesterImpl::construct()
{
    int32_t rc = NO_ERROR;

    if (mConstructed) {
        rc = ALREADY_INITED;
    }

    if (SUCCEED(rc)) {
        rc = mSS.construct();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to construct server state machine, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = mBackupSS.construct();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to construct server state machine, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        mThreads = ThreadPoolEx::getInstance();
        if (ISNULL(mThreads)) {
            LOGE(mModule, "Failed to get thread pool");
            rc = NOT_READY;
        }
    }

    if (SUCCEED(rc)) {
        mConstructed = true;
        LOGD(mModule, "Server tester constructed");
    }

    return rc;
}

int32_t ServerTesterImpl::destruct()
{
    int32_t rc = NO_ERROR;
    int32_t final = NO_ERROR;

    if (!mConstructed) {
        rc = NOT_INITED;
    } else {
        mConstructed = false;
    }

    if (SUCCEED(rc)) {
        rc = mSS.destruct();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to destruct server socket SM, %d", rc);
            final |= rc;
            RESETRESULT(rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = mBackupSS.destruct();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to destruct server socket SM, %d", rc);
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
        LOGE(mModule, "Server tester destructed with error %d", final);
    } else {
        LOGD(mModule, "Server tester destructed");
    }

    return rc;
}

ServerTesterImpl::ServerTesterImpl() :
    mModule(MODULE_SERVER_TESTER),
    mConstructed(false),
    mTestingRequest(REQUEST_TYPE_MAX_INVALID),
    mTestingRound(0),
    mThreads(NULL),
    mSS(TESTER_SOCKET_NAME),
    mBackupSS(TESTER_SOCKET_NAME),
    mPreviewCnt(0),
    mPreviewResult(false),
    mPreviewIdentity(0),
    mPreviewSem(PREVIEW_WAIT_MAX_TIME, 0),
    mNV21PictureCnt(0),
    mNV21PictureResult(false),
    mNV21PictureIdentity(0),
    mNV21PictureSem(NV21_PICTURE_WAIT_MAX_TIME, 0),
    mBayerPictureCnt(0),
    mBayerPictureResult(false),
    mBayerPictureIdentity(0),
    mBayerPictureSem(BAYER_PICTURE_WAIT_MAX_TIME, 0),
    mEventCnt(0),
    mEventResult(false),
    mLastEvent(-1),
    mEventSem(EVENT_WAIT_MAX_TIME, 0),
    mDataCnt(0),
    mDataResult(false),
    mDataSem(CUSTOM_DATA_MAX_WAIT_TIME, 0),
    mMixtureCnt(0),
    mMixtureResult(false),
    mMixtureSem(MIXTURE_MAX_WAIT_TIME, 0)
{
}

ServerTesterImpl::~ServerTesterImpl()
{
    if (mConstructed) {
        destruct();
    }
}

extern ServerTesterImpl gTester;

int32_t cbRequestCFunc(RequestType type, int32_t id, void *header, void *dat)
{
    return gTester.cbRequest(type, id, header, dat);
}

int32_t cbEventCFunc(int32_t event, int32_t arg1, int32_t arg2)
{
    return gTester.cbEvent(event, arg1, arg2);
}

int32_t cbDataCFunc(int32_t type, void *data, int32_t size)
{
    return gTester.cbData(type, data, size);
}

int32_t ServerTesterImpl::resetSirius()
{
    int32_t rc = NO_ERROR;

    if (SUCCEED(rc)) {
        rc = mSirius.abort(PREVIEW_NV21);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to abort preview, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = mSirius.abort(PICTURE_NV21);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to abort nv21 picture, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = mSirius.abort(PICTURE_BAYER);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to abort bayer picture, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = mSirius.abort(EXTENDED_EVENT);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to abort events, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = mSirius.abort(CUSTOM_DATA);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to abort data, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = mSirius.setCallback(cbRequestCFunc);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to set request callback func, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = mSirius.setCallback(cbEventCFunc);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to set event callback func, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = mSirius.setCallback(cbDataCFunc);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to set data callback func, %d", rc);
        }
    }

    return rc;
}

const ServerTesterImpl::cbPreviewFunc ServerTesterImpl::gPreviewCbs[] = {
    [0] = &ServerTesterImpl::cbPreviewRound1,
    [1] = &ServerTesterImpl::cbPreviewRound2,
    [2] = &ServerTesterImpl::cbPreviewRound3,
    [3] = &ServerTesterImpl::cbPreviewRound4,
    [4] = &ServerTesterImpl::cbPreviewRound5,
    [5] = &ServerTesterImpl::cbPreviewRound6,
    [6] = &ServerTesterImpl::cbPreviewRound7,
};

const ServerTesterImpl::cbNV21PictureFunc ServerTesterImpl::gNV21PictureCbs[] = {
    [0] = &ServerTesterImpl::cbNV21PictureRound1,
    [1] = &ServerTesterImpl::cbNV21PictureRound2,
    [2] = &ServerTesterImpl::cbNV21PictureRound3,
    [3] = &ServerTesterImpl::cbNV21PictureRound4,
    [4] = &ServerTesterImpl::cbNV21PictureRound5,
    [5] = &ServerTesterImpl::cbNV21PictureRound6,
    [6] = &ServerTesterImpl::cbNV21PictureRound7,
};

const ServerTesterImpl::cbBayerPictureFunc ServerTesterImpl::gBayerPictureCbs[] = {
    [0] = &ServerTesterImpl::cbBayerPictureRound1,
    [1] = &ServerTesterImpl::cbBayerPictureRound2,
    [2] = &ServerTesterImpl::cbBayerPictureRound3,
    [3] = &ServerTesterImpl::cbBayerPictureRound4,
    [4] = &ServerTesterImpl::cbBayerPictureRound5,
    [5] = &ServerTesterImpl::cbBayerPictureRound6,
    [6] = &ServerTesterImpl::cbBayerPictureRound7,
};

int32_t ServerTesterImpl::cbRequest(RequestType type, int32_t id, void *header, void *dat)
{
    int32_t rc = NO_ERROR;
    SemaphoreTimeout *sem = NULL;

    if (SUCCEED(rc)) {
        if (mTestingRequest != type &&
            mTestingRequest != REQUEST_TYPE_MAX_INVALID) {
            LOGE(mModule, "Received unknown request cb, %s %s",
                getRequestName(type), getRequestName(mTestingRequest));
            rc = BAD_PROTOCAL;
        }
    }

    if (SUCCEED(rc)) {
        if (mTestingRequest == REQUEST_TYPE_MAX_INVALID) {
            mMixtureCnt++;
            rc = cbMixtureRequest(type, id, header, dat);
            if (!SUCCEED(rc)) {
                mMixtureResult = false;
                LOGE(mModule, "Failed to call mixture request callback, %d", rc);
            }

            if (SUCCEED(rc)) {
                rc = mSirius.enqueue(type, id);
                if (!SUCCEED(rc)) {
                    LOGE(mModule, "Failed to enqueue id %d to request %s",
                        id, getRequestName(type));
                } else {
                    rc = JUMP_DONE;
                }
            }

            mMixtureSem.signal();
        }
    }

    if (SUCCEED(rc)) {
        switch (type) {
            case PREVIEW_NV21: {
                PreviewNV21Header *_header = static_cast<PreviewNV21Header *>(header);
                uint8_t *_dat = static_cast<uint8_t *>(dat);
                if (mTestingRequest >= ARRAYSIZE(gPreviewCbs)) {
                    LOGE(mModule, "Not implemented test case, preview callback, "
                        "round %d/%d", mTestingRequest, ARRAYSIZE(gPreviewCbs));
                    rc = BAD_PROTOCAL;
                } else {
                    mPreviewCnt++;
                    rc = (this->*(gPreviewCbs[mTestingRound]))(id, _header, _dat);
                    if (!SUCCEED(rc)) {
                        mPreviewResult = false;
                        LOGE(mModule, "Failed to call preview callback, %d", rc);
                    }
                    sem = &mPreviewSem;
                }
            }; break;
            case PICTURE_NV21: {
                PictureNV21Header *_header = static_cast<PictureNV21Header *>(header);
                uint8_t *_dat = static_cast<uint8_t *>(dat);
                if (mTestingRequest >= ARRAYSIZE(gNV21PictureCbs)) {
                    LOGE(mModule, "Not implemented test case, nv21 picture callback, "
                        "round %d/%d", mTestingRequest, ARRAYSIZE(gNV21PictureCbs));
                    rc = BAD_PROTOCAL;
                } else {
                    mNV21PictureCnt++;
                    rc = (this->*(gNV21PictureCbs[mTestingRound]))(id, _header, _dat);
                    if (!SUCCEED(rc)) {
                        mNV21PictureResult = false;
                        LOGE(mModule, "Failed to call nv21 picture callback, %d", rc);
                    }
                    sem = &mNV21PictureSem;
                }
            }; break;
            case PICTURE_BAYER: {
                PictureBayerHeader *_header = static_cast<PictureBayerHeader *>(header);
                uint8_t *_dat = static_cast<uint8_t *>(dat);
                if (mTestingRequest >= ARRAYSIZE(gBayerPictureCbs)) {
                    LOGE(mModule, "Not implemented test case, bayer picture callback, "
                        "round %d/%d", mTestingRequest, ARRAYSIZE(gBayerPictureCbs));
                    rc = BAD_PROTOCAL;
                } else {
                    mBayerPictureCnt++;
                    rc = (this->*(gBayerPictureCbs[mTestingRound]))(id, _header, _dat);
                    if (!SUCCEED(rc)) {
                        mBayerPictureResult = false;
                        LOGE(mModule, "Failed to call bayer picture callback, %d", rc);
                    }
                    sem = &mBayerPictureSem;
                }
            }; break;
            default: {
                LOGE(mModule, "Unknown request cb, %s %s",
                    getRequestName(type), getRequestName(mTestingRequest));
            }; break;
        }
    }

    if (SUCCEED(rc)) {
        rc = mSirius.enqueue(type, id);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to enqueue id %d to request %s",
                id, getRequestName(type));
        }
    }

    if (SUCCEED(rc)) {
        sem->signal();
    }

    return RETURNIGNORE(rc, JUMP_DONE);
}

const ServerTesterImpl::cbEventFunc ServerTesterImpl::gEventCbs[] = {
    [0] = &ServerTesterImpl::cbEventRound1,
    [1] = &ServerTesterImpl::cbEventRound2,
    [2] = &ServerTesterImpl::cbEventRound3,
    [3] = &ServerTesterImpl::cbEventRound4,
    [4] = &ServerTesterImpl::cbEventRound5,
    [5] = &ServerTesterImpl::cbEventRound6,
    [6] = &ServerTesterImpl::cbEventRound7,
};

int32_t ServerTesterImpl::cbEvent(int32_t event, int32_t arg1, int32_t arg2)
{
    int32_t rc = NO_ERROR;

    if (SUCCEED(rc)) {
        if (mTestingRequest != EXTENDED_EVENT &&
            mTestingRequest != REQUEST_TYPE_MAX_INVALID) {
            LOGE(mModule, "Received unknown request cb, %s %s",
                getRequestName(EXTENDED_EVENT),
                getRequestName(mTestingRequest));
            rc = BAD_PROTOCAL;
        }
    }

    if (SUCCEED(rc)) {
        if (mTestingRequest == REQUEST_TYPE_MAX_INVALID) {
            mMixtureCnt++;
            mMixtureSem.signal();
            rc = cbMixtureEvent(event, arg1, arg2);
            if (!SUCCEED(rc)) {
                mMixtureResult = false;
                LOGE(mModule, "Failed to call mixture request callback, %d", rc);
            } else {
                rc = JUMP_DONE;
            }
        }
    }

    if (SUCCEED(rc)) {
        if (mTestingRequest >= ARRAYSIZE(gEventCbs)) {
            LOGE(mModule, "Not implemented test case, events, round %d/%d",
                mTestingRequest, ARRAYSIZE(gEventCbs));
            rc = BAD_PROTOCAL;
        }
    }

    if (SUCCEED(rc)) {
        mEventCnt++;
        mEventSem.signal();
        rc = (this->*(gEventCbs[mTestingRound]))(event, arg1, arg2);
        if (!SUCCEED(rc)) {
            mEventResult = false;
            LOGE(mModule, "Failed to call cb of events, round %d, %d",
                mTestingRound, rc);
        }
    }

    return RETURNIGNORE(rc, JUMP_DONE);
}

const ServerTesterImpl::cbDataFunc ServerTesterImpl::gDataCbs[] = {
    [0] = &ServerTesterImpl::cbDataRound1,
    [1] = &ServerTesterImpl::cbDataRound2,
    [2] = &ServerTesterImpl::cbDataRound3,
    [3] = &ServerTesterImpl::cbDataRound4,
    [4] = &ServerTesterImpl::cbDataRound5,
    [5] = &ServerTesterImpl::cbDataRound6,
    [6] = &ServerTesterImpl::cbDataRound7,
    [7] = &ServerTesterImpl::cbDataRound8,
    [8] = &ServerTesterImpl::cbDataRound9,
    [9] = &ServerTesterImpl::cbDataRound10,
    [10] = &ServerTesterImpl::cbDataRound11,
    [11] = &ServerTesterImpl::cbDataRound12,
    [12] = &ServerTesterImpl::cbDataRound13,
    [13] = &ServerTesterImpl::cbDataRound14,
    [14] = &ServerTesterImpl::cbDataRound15,
    [15] = &ServerTesterImpl::cbDataRound16,
};

int32_t ServerTesterImpl::cbData(int32_t type, void *data, int32_t size)
{
    int32_t rc = NO_ERROR;

    if (mTestingRequest != CUSTOM_DATA &&
        mTestingRequest != REQUEST_TYPE_MAX_INVALID) {
        LOGE(mModule, "Received unknown request cb, %s %s",
            getRequestName(CUSTOM_DATA),
            getRequestName(mTestingRequest));
        rc = BAD_PROTOCAL;
    }

    if (SUCCEED(rc)) {
        if (mTestingRequest == REQUEST_TYPE_MAX_INVALID) {
            mMixtureCnt++;
            mMixtureSem.signal();
            rc = cbMixtureData(type, data, size);
            if (!SUCCEED(rc)) {
                mMixtureResult = false;
                LOGE(mModule, "Failed to call mixture request callback, %d", rc);
            } else {
                rc = JUMP_DONE;
            }
        }
    }

    if (SUCCEED(rc)) {
        if ((uint32_t)mTestingRound >= ARRAYSIZE(gDataCbs)) {
            LOGE(mModule, "Not implemented test case, custom data, "
                "round %d/%d", mTestingRequest, ARRAYSIZE(gDataCbs));
            rc = BAD_PROTOCAL;
        }
    }

    if (SUCCEED(rc)) {
        mDataCnt++;
        mDataSem.signal();
        rc = (this->*(gDataCbs[mTestingRound]))(type, data, size);
        if (!SUCCEED(rc)) {
            mDataResult = false;
            LOGE(mModule, "Failed to call cb of events, round %d, %d",
                mTestingRound, rc);
        }
    }

    return RETURNIGNORE(rc, JUMP_DONE);
}

};

