#include "configuration.h"
#include "RequestHandler.h"
#include "MemMgmt.h"

namespace sirius {

RequestType RequestHandler::getType()
{
    return mType;
}

const char *RequestHandler::getName()
{
    return mName;
}

RequestHandler::RequestHandler(HandlerOpsIntf *ops,
    RequestType type, const char *name, uint32_t memNum) :
    mConstructed(false),
    mModule(MODULE_REQUEST_HANDLER),
    mName(name),
    mType(type),
    mMemShared(false),
    mMemNum(memNum),
    mMem(NULL),
    mOps(ops)
{
    ASSERT_LOG(mModule, NOTNULL(ops), "Ops shouldn't be NULL");
    ASSERT_LOG(mModule, memNum > 0,  "Mem num shoudn't be 0");
    ASSERT_LOG(mModule, mMemNum < REQUEST_HANDLER_MAX_MEMORY_NUM,
        "Too much mem to share, %d/%d", mMemNum, REQUEST_HANDLER_MAX_MEMORY_NUM);
    ASSERT_LOG(mModule, getRequestType(type) != REQUEST_TYPE_MAX_INVALID,
        "Invalid request type %d", type);
    if (ISNULL(name)) {
        mName = "generic request handler";
    }
}

RequestHandler::~RequestHandler()
{
    if (mConstructed) {
        destruct();
    }
}

int32_t RequestHandler::construct()
{
    int32_t rc = NO_ERROR;

    if (mConstructed) {
        rc = ALREADY_INITED;
    }

    if (SUCCEED(rc)) {
        rc = mSSSM.construct();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to construct ssm, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        mConstructed = true;
        LOGD(mModule, "%s constructed", getName());
    }

    return rc;
}

int32_t RequestHandler::destruct()
{
    int32_t rc = NO_ERROR;
    int32_t final = NO_ERROR;

    if (!mConstructed) {
        rc = NOT_INITED;
    } else {
        mConstructed = false;
    }

    if (SUCCEED(rc)) {
        if (NOTNULL(mThreads)) {
            mThreads->removeInstance();
            mThreads = NULL;
        }
    }

    if (SUCCEED(rc)) {
        rc = mSSSM.destruct();
        if (!SUCCEED(rc)) {
            final |= rc;
            LOGE(mModule, "Failed to destruct ssm, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = releaseMem();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to release all buffers, %d", rc);
            final |= rc;
            rc = NO_ERROR;
        }
    }

    if (!SUCCEED(final)) {
        LOGE(mModule, "%s destructed with error %d", getName(), rc);
    } else {
        LOGD(mModule, "%s destructed", getName());
    }

    return final;
}

int32_t RequestHandler::setSocketFd(int32_t fd)
{
    int32_t rc = NO_ERROR;

    if (SUCCEED(rc)) {
        rc = mSSSM.setClientFd(fd);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to construct ssm, %d", rc);
        }
    }

    return rc;
};

int32_t RequestHandler::allocMem()
{
    int32_t rc = NO_ERROR;
    void *mem = NULL;
    int32_t fd = -1;
    int32_t size = getExpectedBufferSize();

    if (!ISNULL(mMem)) {
        LOGE(mModule, "Already alloced memory");
        rc = ALREADY_INITED;
    }

    if (SUCCEED(rc)) {
        if (size == 0) {
            rc = NOT_REQUIRED;
        }
    }

    if (SUCCEED(rc)) {
        mMem = (MemoryInfo *)Malloc(sizeof(MemoryInfo) * mMemNum);
        if (ISNULL(mMem)) {
            LOGE(mModule, "Failed to alloc memory");
            rc = NO_MEMORY;
        }
    }

    if (SUCCEED(rc)) {
        for (uint32_t i = 0; i < mMemNum; i++) {
            mMem[i].id   = -1;
            mMem[i].buf  = NULL;
            mMem[i].size = 0;
            mMem[i].serverfd = -1;
            mMem[i].clientfd = -1;
        }
    }

    if (SUCCEED(rc)) {
        for (uint32_t i = 0; i < mMemNum; i++) {
            mem = NULL; fd = -1;
            rc = mOps->allocateBuf(&mem, size, &fd);
            if (!SUCCEED(rc)) {
                LOGE(mModule, "Failed to alloc %dB memory, %d", size, rc);
                break;
            } else {
                LOGD(mModule, "Succeed to alloc %dB memory", size);
                mMem[i].id   = i;
                mMem[i].buf  = mem;
                mMem[i].size = size;
                mMem[i].serverfd = fd;
            }
        }
    }

    return rc;
}

int32_t RequestHandler::releaseMem()
{
    int32_t rc = NO_ERROR;

    if (ISNULL(mMem)) {
        if (getExpectedBufferSize()) {
            LOGE(mModule, "Memory not allocated.");
            rc = NOT_INITED;
        } else {
            rc = NOT_REQUIRED;
        }
    }

    if (SUCCEED(rc)) {
        for (uint32_t i = 0; i < mMemNum; i++) {
            if (NOTNULL(mMem[i].buf)) {
                void *buf = mMem[i].buf;
                mMem[i].id   = -1;
                mMem[i].buf  = NULL;
                mMem[i].size = 0;
                mMem[i].serverfd = -1;
                mMem[i].clientfd = -1;
                rc = mOps->releaseBuf(buf);
                if (!SUCCEED(rc)) {
                    LOGF(mModule, "Failed to release memory, will cause "
                        "%d bytes memory leakage, %d", mMem[i].size, rc);
                }
            }
        }
    }

    if (SUCCEED(rc)) {
        if (NOTNULL(mMem)) {
            SECURE_FREE(mMem);
        }
    }

    return RETURNIGNORE(rc, NOT_REQUIRED);
}

int32_t RequestHandler::shareSingleMem(int32_t fd)
{
    int32_t rc = NO_ERROR;
    char msg[SOCKET_DATA_MAX_LEN];
    int32_t clientfd = -1;

    if (SUCCEED(rc)) {
        sprintf(msg, SOCKET_SERVER_SHARE_STR " %d", getType());
        rc = mSSSM.sendMsg(msg, strlen(msg));
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to send msg %s, %d", msg, rc);
        }
    }

    if (SUCCEED(rc)) {
        msg[0] = '\0';
        rc = mSSSM.receiveMsg(msg, SOCKET_DATA_MAX_LEN);
        msg[SOCKET_DATA_MAX_LEN - 1] = '\0';
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to receive clent msg, %d", msg);
        }
    }

    if (SUCCEED(rc)) {
        if (strncmp(msg, SOCKET_CLIENT_REPLY_SHARE_STR,
            strlen(SOCKET_CLIENT_REPLY_SHARE_STR))) {
            LOGE(mModule, "Client error occuried when sharing memory,"
                " msg=\"%s\", %d", msg, rc);
            rc = UNKNOWN_ERROR;
        }
    }

    if (SUCCEED(rc)) {
        rc = mSSSM.sendFd(fd);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to send client fd %d, %d", fd, rc);
        }
    }

    if (SUCCEED(rc)) {
        msg[0] = '\0';
        rc = mSSSM.receiveMsg(msg, SOCKET_DATA_MAX_LEN);
        msg[SOCKET_DATA_MAX_LEN - 1] = '\0';
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to receive clent msg, %d", msg);
        }
    }

    if (SUCCEED(rc)) {
        if (strncmp(msg, SOCKET_CLIENT_REPLY_FD_STR,
            strlen(SOCKET_CLIENT_REPLY_FD_STR))) {
            LOGE(mModule, "Client error occuried when sharing memory,"
                " msg=\"%s\", %d", msg, rc);
            rc = UNKNOWN_ERROR;
        }
    }

    if (SUCCEED(rc)) {
        rc = convertToClientFd(msg, SOCKET_CLIENT_REPLY_FD_STR, &clientfd);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to convert to client fd, %d", rc);
        }
        if (clientfd <= 0) {
            LOGE(mModule, "Invalid client fd %d", clientfd);
            rc = UNKNOWN_ERROR;
        }
    }

    if (SUCCEED(rc)) {
        rc = recordMemoryPair(fd, clientfd);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to record memory pair");
        }
    }

    if (SUCCEED(rc)) {
        rc = mOps->addMemory(getType(), clientfd, USED_MEMORY);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to add memory to controller, %d", rc);
        }
     }

    return rc;
}

int32_t RequestHandler::convertToClientFd(
    char *msg, const char *prefix, int32_t *clientfd)
{
    int32_t rc = NO_ERROR;

    if (!COMPARE_SAME_STRING(msg, prefix, strlen(prefix))) {
        LOGE(mModule, "Prefix not match, %s VS %s", msg, prefix);
        rc = PARAM_INVALID;
    }

    if (!SUCCEED(rc)) {
        *clientfd = atoi(msg + strlen(prefix) + 1);
        if (*clientfd <= 0) {
            LOGE(mModule, "Invalid msg, \"%s\"", msg);
            rc = NOT_FOUND;
        }
    }

    return rc;
}

int32_t RequestHandler::recordMemoryPair(int32_t serverfd, int32_t clientfd)
{
    int32_t rc = NO_ERROR;
    uint32_t i = 0;

    if (SUCCEED(rc)) {
        for (i = 0; i < mMemNum; i++) {
            if (mMem[i].serverfd == serverfd) {
                mMem[i].clientfd = clientfd;
                break;
            }
        }
    }

    if (i == mMemNum &&
        mMem[i - 1].serverfd != serverfd) {
        LOGE(mModule, "Not found memory pair.");
        rc = UNKNOWN_ERROR;
    }

    return rc;
}

int32_t RequestHandler::shareMem()
{
    int32_t rc = NO_ERROR;

    if (SUCCEED(rc)) {
        for (uint32_t i = 0; i < mMemNum; i++) {
            rc = shareSingleMem(mMem[i].serverfd);
            if (!SUCCEED(rc)) {
                LOGE(mModule, "Failed to share type %s fd %d",
                    getName(), mMem[i].serverfd);
            }
        }
    }

    if (SUCCEED(rc)) {
        rc = mSSSM.sendMsg(SOCKET_SERVER_SHARE_DONE,
            strlen(SOCKET_SERVER_SHARE_DONE));
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to send share done msg, %d", rc);
        }
    }

    return rc;
}

int32_t RequestHandler::allocMemAndShare()
{
    int32_t rc = NO_ERROR;

    if (SUCCEED(rc)) {
        rc = allocMem();
        if (rc == NOT_REQUIRED) {
            LOGE(mModule, "Request %d not required memory", getType());
        } else if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to alloc memory, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = shareMem();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to share memory, %d", rc);
        } else {
            mMemShared = true;
        }
    }

    return RETURNIGNORE(rc, NOT_REQUIRED);
}

int32_t RequestHandler::onClientReady()
{
    int32_t rc = NO_ERROR;

    if (SUCCEED(rc)) {
        mThreads = ThreadPoolEx::getInstance();
        if (ISNULL(mThreads)) {
            LOGE(mModule, "Failed to get thread pool");
            rc = UNKNOWN_ERROR;
        }
    }

    if (SUCCEED(rc)) {
        rc = mThreads->run(
            [this]() -> int32_t {
                return startServerLoop();
            }
        );
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to run on thread, %d", rc);
        }
    }

    return rc;
}

int32_t RequestHandler::startServerLoop()
{
    int32_t rc = NO_ERROR;

    if (SUCCEED(rc)) {
        rc = mOps->getHeader(mHeader);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to set memory size %d, %d", size, rc);
        }
    }

    if (SUCCEED(rc)) {
        int32_t size = getExpectedBufferSize();
        rc = mOps->setMemSize(getType(), size);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to set memory %dB to %s, %d", size, )
        }
    }

    if (SUCCEED(rc)) {
        rc = allocMemAndShare();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to share mem with client, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        do {
            int32_t fd = 0;
            RESETRESULT(rc);
            char msg[SOCKET_DATA_MAX_LEN];
            if (SUCCEED(rc)) {
                msg[0] = '\0';
                rc = mSSSM.receiveMsg(msg, SOCKET_DATA_MAX_LEN);
                msg[SOCKET_DATA_MAX_LEN - 1] = '\0';
                if (rc == USER_ABORTED) {
                    LOGI(mModule, "Abort to read client msg.");
                } else if (!SUCCEED(rc)) {
                    LOGE(mModule, "Failed to receive clent msg, %d", msg);
                }
            }

            if (SUCCEED(rc)) {
                rc = convertToClientFd(msg, SOCKET_CLIENT_NOTIFIER_STR, &fd);
                if (!SUCCEED(rc)) {
                    LOGE(mModule, "Failed to convert, %s", msg);
                }
            }

            if (SUCCEED(rc)) {
                rc = onMemRefreshed(fd);
                if (!SUCCEED(rc)) {
                    LOGE(mModule, "Failed to process freshed memory, %d", rc);
                }
            }
        } while (rc != USER_ABORTED);
    }

    return rc;
}

int32_t RequestHandler::onMemRefreshed(int32_t fd)
{
    int32_t rc = NO_ERROR;
    int32_t id = -1;
    void *buf  = NULL;
    void *head = NULL;
    void *dat  = NULL;

    if (SUCCEED(rc)) {
        for (int32_t i = 0; i < mMemNum; i++) {
            if (mMem[i].clientfd == fd) {
                id  = mMem[i].id;
                buf = mMem[i].buf;
                break;
            }
        }
        if (id == -1) {
            LOGE(mModule, "Invalid memory fd %d", fd);
            rc = NOT_FOUND;
        }
    }

    if (SUCCEED(rc)) {
        bool fresh = USED_MEMORY;
        rc = mOps->getMemStatus(getType(), fd, &fresh);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to get mem status, %d.", rc);
        }
        if (fresh != FRESH_MEMORY) {
            LOGE(mModule, "Invalid mem status, should be fresh.");
            rc = NOT_EXIST;
        }
    }

    if (SUCCEED(rc)) {
        int32_t size = getHeaderSize();
        if (size > 0) {
            head = buf;
            dat  = (void *)((int8_t *)buf + size);
        } else {
            LOGE(mModule, "Invalid header size, %d", size);
            rc = NOT_READY;
        }
    }

    if (SUCCEED(rc)) {
        rc = mOps->send(getType(), id, head, dat);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to send callback, %d", rc);
        }
    }

    return rc;
}

int32_t RequestHandler::enqueue(int32_t id)
{
    int32_t rc = NO_ERROR;
    int32_t clientFd = -1;

    if (SUCCEED(rc)) {
        for (uint32_t i = 0; i < mMemNum; i++) {
            if (mMem[i].id == id) {
                clientFd = mMem[i].clientfd;
                break;
            }
        }
        if (clientFd == -1) {
            LOGE(mModule, "Failed to find enqueue buf, %d", id);
            rc = NOT_FOUND;
        }
    }

    if (SUCCEED(rc)) {
        bool fresh = USED_MEMORY;
        rc = mOps->getMemStatus(getType(), clientFd, &fresh);
        if (SUCCEED(rc)) {
            if (fresh == USED_MEMORY) {
                LOGE(mModule, "Memory already enqueued.");
                rc = NOT_REQUIRED;
            }
        }
    }

    if (SUCCEED(rc)) {
        rc = mOps->setMemStatus(getType(), clientFd, USED_MEMORY);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to set memory status, %d", rc);
        }
    }

    return rc;
}

int32_t RequestHandler::abort()
{
    int32_t rc = NO_ERROR;

    if (ISNULL(mThreads)) {
        LOGE(mModule, "Server thread alreay exited.");
        rc = NOT_EXIST;
    }

    if (SUCCEED(rc)) {
        rc = exitServerLoop();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to abort thread loop, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        if (NOTNULL(mThreads)) {
            mThreads->removeInstance();
            mThreads = NULL;
        }
    }

    return final;
}

int32_t RequestHandler::getExpectedBufferSize()
{
    return getHeaderSize() + getDataSize();
}

int32_t RequestHandler::exitServerLoop()
{
    int32_t rc = mSSSM.cancelWaitMsg();
    if (!SUCCEED(rc)) {
        LOGE(mModule, "Failed to cancel waitting msg, %d");
    }

    return rc;
}

};

