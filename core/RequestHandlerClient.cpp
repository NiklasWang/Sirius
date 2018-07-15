#include "RequestHandlerClient.h"
#include "MemMgmt.h"

namespace sirius {

RequestHandlerClient::RequestHandlerClient(RequestType type, const char *name, uint32_t maxNum) :
    mConstructed(false),
    mModule(MODULE_REQUEST_CLIENT_HANDLER),
    mType(type),
    mName(name),
    mConnected(false),
    mReady(false),
    mThreads(NULL),
    mMemNum(0),
    mMemMaxNum(maxNum),
    mMemMap(NULL)
{
    ASSERT_LOG(mModule, maxNum > 0,  "Max mem num should > 0");
    ASSERT_LOG(mModule, maxNum <= REQUEST_HANDLER_MAX_MEMORY_NUM,
        "Too much mem to share, %d/%d", maxNum, REQUEST_HANDLER_MAX_MEMORY_NUM);
    ASSERT_LOG(mModule, getRequestType(type) != REQUEST_TYPE_MAX_INVALID,
        "Invalid request type %d", type);
    if (ISNULL(name)) {
        mName = "generic request client handler";
    }
    pthread_mutex_init(&mLocker, NULL);
}

RequestHandlerClient::~RequestHandlerClient()
{
    if (mConstructed) {
        destruct();
    }
    pthread_mutex_destroy(&mLocker);
}

const char *RequestHandlerClient::getName()
{
    return mName;
}

RequestType RequestHandlerClient::getType()
{
    return mType;
}

int32_t RequestHandlerClient::construct()
{
    int32_t rc = NO_ERROR;

    if (mConstructed) {
        rc = ALREADY_INITED;
    }

    if (ISNULL(mMemMap)) {
        mMemMap = (MemoryMap *)Malloc(sizeof(MemoryMap) * mMemMaxNum);
        if (ISNULL(mMemMap)) {
            LOGE(mModule, "Failed to alloc %d bytes.",
                sizeof(MemoryMap) * mMemMaxNum);
        } else {
            for (int32_t i = 0; i < mMemMaxNum; i++) {
                mMemMap[i].fd   = -1;
                mMemMap[i].mem  = NULL;
                mMemMap[i].size = 0;
            }
        }
    }

    if (SUCCEED(rc)) {
        rc = mSC.construct();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to construct socket state machine, %d", rc);
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
        LOGD(mModule, "Request client handler constructed");
    }

    return rc;
}

int32_t RequestHandlerClient::destruct()
{
    int32_t rc = NO_ERROR;
    int32_t final = rc;

    if (!mConstructed) {
        rc = NOT_INITED;
    } else {
        mConstructed = false;
    }

    if (SUCCEED(rc)) {
        mReady = false;
        mConnected = false;
    }

    if (SUCCEED(rc)) {
        mSC.cancelWaitMsg();
        rc = mSC.destruct();
        if (!SUCCEED(rc)) {
            final |= rc;
            LOGE(mModule, "Failed to destruct socket state machine, %d", rc);
            RESETRESULT(rc);
        }
    }

    if (SUCCEED(rc)) {
        if (NOTNULL(mThreads)) {
            mThreads->removeInstance();
            mThreads = NULL;
        }
    }

    if (SUCCEED(rc)) {
        rc = releaseAllMems();
        if (!SUCCEED(rc)) {
            final |= rc;
            LOGE(mModule, "Failed to release all mems, %d", rc);
            RESETRESULT(rc);
        }
    }

    if (SUCCEED(rc)) {
        if (NOTNULL(mMemMap)) {
            SECURE_FREE(mMemMap);
        }
    }

    if (!SUCCEED(final)) {
        LOGE(mModule, "Request client handler %s destructed "
            "with error %d", getName(), final);
    } else {
        LOGD(mModule, "Request client handler %s destructed", getName());
    }

    return rc;
}

int32_t RequestHandlerClient::prepare()
{
    int32_t rc = NO_ERROR;
    bool locked  = false;

    if (mReady) {
        LOGE(mModule, "Already prepared.");
        rc = ALREADY_INITED;
    }

    if (SUCCEED(rc)) {
        pthread_mutex_lock(&mLocker);
        locked = true;
        if (mReady) {
            rc = ALREADY_INITED;
        }
    }
    
    if (SUCCEED(rc)) {
        if (!mCore.ready()) {
            rc = mCore.prepare();
            if (!SUCCEED(rc)) {
                LOGI(mModule, "Failed to prepare sirius client core, "
                    "Server may not started, %s %d", strerror(errono), rc);
            }
        }
    }

    if (SUCCEED(rc)) {
        if (!mConnected) {
            rc = mSC.connectServer();
            if (!SUCCEED(rc)) {
                LOGD(mModule, "Failed to connect server, "
                    "may not started, %s %d", strerror(errono), rc);
                rc = NOT_EXIST;
            } else {
                mConnected = true;
            }
        }
    }

    if (SUCCEED(rc)) {
        char msg[SOCKET_DATA_MAX_LEN];
        sprintf(msg, SOCKET_CLIENT_CONNECT_TYPE " %d", getType());
        rc = mSC.sendMsg(SOCKET_CLIENT_CONNECT_TYPE, strlen(msg));
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to send msg \"%s\" to server, %d", msg, rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = syncServerMemory();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to sync server memory, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        mReady = true;
    }

    if (SUCCEED(rc) || !SUCCEED(rc)) {
        if (locked) {
            pthread_mutex_unlock(&mLocker);
        }
    }

    return rc;
}

int32_t RequestHandlerClient::syncServerMemory()
{
    int32_t rc = NO_ERROR;
    int32_t final = rc;
    char msg[SOCKET_DATA_MAX_LEN];

    for (int32_t i = 0; i < mMemMaxNum; i++) {
        rc = acceptSingleMemory();
        if (rc == JUMP_DONE) {
            break;
        } else if (!SUCCEED(rc)) {
            final |= rc;
            LOGE(mModule, "Failed to accept single memory %d, %d", i, rc);
        } else {
            mMemNum++;
        }
    }

    return final;
}

int32_t RequestHandlerClient::acceptSingleMemory()
{
    int32_t rc = NO_ERROR;
    int32_t clientfd = -1;
    void *buf = NULL;
    int32_t size = -1;
    char msg[SOCKET_DATA_MAX_LEN];

    if (SUCCEED(rc)) {
        msg[0] = '\0';
        rc = mSC.receiveMsg(msg, SOCKET_DATA_MAX_LEN);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to receive msg, %d", rc);
        }
    }
    
    if (SUCCEED(rc)) {
        if (COMPARE_SAME_LEN_STRING(msg, SOCKET_SERVER_SHARE_DONE,
            strlen(SOCKET_SERVER_SHARE_DONE))) {
            LOGD(mModule, "Memory share done.");
            rc = JUMP_DONE;
        }
    }

    if (SUCCEED(rc)) {
        RequestType result = REQUEST_TYPE_MAX_INVALID;
        rc = convertToRequestType(msg, SOCKET_SERVER_SHARE_STR, result);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to convert to request type, %s -> %s, %d",
                msg, SOCKET_SERVER_SHARE_STR, rc);
        }
        if (result != getType()) {
            LOGE(mModule, "Shouldn't be here, bad request type, %d/%d",
                result, getType());
            rc = BAD_PROTOCAL;
        }
    }

    if (SUCCEED(rc)) {
        rc = mSC.sendMsg(SOCKET_CLIENT_REPLY_SHARE_STR,
            strlen(SOCKET_CLIENT_REPLY_SHARE_STR));
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to send msg to server, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = mSC.receiveFd(&clientfd);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to receive fd from server, %d", rc);
        }
        if (clientfd == -1) {
            LOGE(mModule, "Invalid fd -1 from server.");
            rc = INVALID_FORMAT;
        }
    }

    if (SUCCEED(rc)) {
        sprintf(msg, SOCKET_CLIENT_REPLY_FD_STR " %d", clientfd);
        rc = mSC.sendMsg(msg, strlen(msg));
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to send msg to server, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = mCore.getMemSize(getType(), &size);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to send msg to server, %d", rc);
        }
        if (size <= 0) {
            LOGE(mModule, "Failed to get memory size");
            rc = BAD_PROTOCAL;
        }
    }

    if (SUCCEED(rc)) {
        rc = mCore.importBuf(&buf, clientfd, size);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to import memory for fd %d size %d, %d",
                clientfd, size, rc);
        } else {
            LOGD(mModule, "Request type %d memory fd %d mapped %p",
                getType(), clientfd, buf);
        }
        if (ISNULL(buf)) {
            LOGE(mModule, "Buf is NULL and that's invalid");
            rc = UNKNOWN_ERROR;
        }
    }

    if (SUCCEED(rc)) {
        rc = addMemoryMap(buf, clientfd, size);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to record memory map, %d", rc);
            mCore.releaseBuf(buf);
        }
    }

    return rc;
}

int32_t RequestHandlerClient::convertToRequestType(
    char *msg, const char *prefix, RequestType &result)
{
    int32_t rc = NO_ERROR;
    int32_t type = -1;

    if (!COMPARE_SAME_STRING(msg, prefix, strlen(prefix))) {
        LOGE(mModule, "Prefix not match, %s VS %s", msg, prefix);
        rc = PARAM_INVALID;
    }

    if (!SUCCEED(rc)) {
        type = atoi(msg + strlen(prefix) + 1);
        if (type <= 0) {
            LOGE(mModule, "Invalid msg, \"%s\"", msg);
            rc = NOT_FOUND;
        } else {
            result = gRequestTypeMap[type];
        }
    }

    return rc;
}


bool RequestHandlerClient::requested()
{
    int32_t rc  = NO_ERROR;
    bool result = false;

    if (SUCCEED(rc)) {
        if (!mCore.ready()) {
            rc = mCore.prepare();
            if (!SUCCEED(rc)) {
                LOGI(mModule, "Failed to prepare sirius client core, %d", rc);
            }
        }
    }

    if (SUCCEED(rc)) {
        if (mCore.requested(getType())) {
            result = true;
        }
    }

    return result;
}

int32_t RequestHandlerClient::addMemoryMap(void *mem, int32_t fd, int32_t size)
{
    int32_t rc = NOT_FOUND;

    for (int32_t i = 0; i < mMemMaxNum; i++) {
        if (mMemMap[i].fd == -1) {
            mMemMap[i].mem = mem;
            mMemMap[i].fd  = fd;
            mMemMap[i].size = size;
            rc = NO_ERROR;
            break;
        }
    }

    return rc;
}

int32_t RequestHandlerClient::findMemoryMap(int32_t fd, void **mem, int32_t *size)
{
    int32_t rc = NOT_FOUND;
    void *result = NULL;
    int32_t maxSize = -1;

    for (int32_t i = 0; i < mMemNum; i++) {
        if (mMemMap[i].fd == fd &&
            mMemMap[i].fd != -1) {
            result = mMemMap[i].mem;
            maxSize = mMemMap[i].size;
            rc = NO_ERROR;
            break;
        }
    }

    *mem = result;
    *size = maxSize;

    return rc;
}

int32_t RequestHandlerClient::releaseAllMems()
{
    int32_t rc = NO_ERROR;

    for (int32_t i = 0; i < mMemNum; i++) {
        if (mMemMap[i].mem != NULL) {
            mCore.releaseBuf(mMemMap[i].mem);
            mMemMap[i].fd = -1;
            mMemMap[i].mem = NULL;
            mMemMap[i].size = 0;
        }
    }

    return rc;
}

int32_t RequestHandlerClient::onDataReady(void *header, uint8_t *dat)
{
    int32_t rc = NO_ERROR;
    int32_t fd = -1;
    void *mem = NULL;
    int32_t maxSize = -1;

    if (SUCCEED(rc)) {
        if (!requested()) {
            LOGD(mModule, "%d not requested, abandon this time.", getType());
            rc = NOT_REQUIRED;
        }
    }

    if (SUCCEED(rc)) {
        if (!mReady) {
            rc = prepare();
            if (!SUCCEED(rc)) {
                LOGE(mModule, "Failed to prepare request client handler, %d", rc);
            }
        }
    }

    if (SUCCEED(rc)) {
        rc = mCore.getUsedMem(getType(), &fd);
        if (rc == NOT_FOUND) {
            LOGI(mModule, "Sirius server memory all full, skip this frame");
        } else if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to get used memory, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = findMemoryMap(fd, &mem, &maxSize);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to find memory map, %d", rc);
        }
        if (ISNULL(mem)) {
            LOGE(mModule, "Didn't find memory in map");
            rc = NOT_FOUND;
        }
    }

    if (SUCCEED(rc)) {
        int32_t headsize = sizeOfHeader();
        int32_t datasize = sizeOfData(header);
        if (maxSize < (headsize + datasize)) {
            LOGE(mModule, "Insuffciant server memory size for %d "
                "request, %d / %d + %d", getType(), maxSize, headsize, datasize);
            rc = BAD_PROTOCAL;
        } else {
            memcpy(mem, header, headsize);
        }
    }

    if (SUCCEED(rc)) {
        rc = copyDataToServer((uint8_t *)mem + sizeOfHeader(), dat);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to copy to server, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = mCore.setMemStatus(getType(), fd, FRESH_MEMORY);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to renew memory status, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = notifyDataReady(fd);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to send data notify to server, %d", rc);
        }
    }

    return rc;
}

int32_t RequestHandlerClient::notifyDataReady(int32_t fd)
{
    int32_t rc = NO_ERROR;
    char msg[SOCKET_DATA_MAX_LEN];

    if (SUCCEED(rc)) {
        sprintf(msg, SOCKET_CLIENT_NOTIFIER_STR " %d", fd);
        rc = mSC.sendMsg(msg, strlen(msg));
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to send msg to server, %d", rc);
        }
    }

    return rc;
}

const RequestType SiriusCore::gRequestTypeMap[] = {
    [PREVIEW_NV21]   = PREVIEW_NV21,
    [PICTURE_NV21]   = PICTURE_NV21,
    [PICTURE_BAYER]  = PICTURE_BAYER,
    [EXTENDED_EVENT] = EXTENDED_EVENT,
    [CUSTOM_DATA]    = CUSTOM_DATA,
    [REQUEST_TYPE_MAX_INVALID] = REQUEST_TYPE_MAX_INVALID,
};

};

