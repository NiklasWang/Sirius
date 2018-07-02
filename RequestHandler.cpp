#include "configuration.h"
#include "RequestHandler.h"

namespace sirius {

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
        if (NOTNULL(mRunOnce)) {
            rc = abort();
            if (!SUCCEED(rc)) {
                LOGE(mModule, "Failed to exit run once thread.");
                final |= rc;
                rc = NO_ERROR;
            }
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
        mMem = (MemoryInfo *)malloc(sizeof(MemoryInfo) * mMemNum);
        if (ISNULL(mMem)) {
            LOGE(mModule, "Failed to alloc memory");
            rc = NO_MEMORY;
        }
    }

    if (SUCCEED(rc)) {
        for (uint32_t i = 0; i < mMemNum; i++) {
            mMem[i].buf = NULL;
            mMem[i].size = 0;
            mMem[i].serverfd = -1;
            mMem[i].clientfd = -1;
        }
    }

    if (SUCCEED(rc)) {
        for (uint32_t i = 0; i < mMemNum; i++) {
            mem = NULL; fd = -1;
            rc = mOps->allocateIon(&mem, size, &fd);
            if (!SUCCEED(rc)) {
                LOGE(mModule, "Failed to alloc %dB memory, %d", size, rc);
                break;
            } else {
                LOGD(mModule, "Succeed to alloc %dB memory", size);
                mMem[i].buf = mem;
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
        LOGE(mModule, "Memory not allocated.");
        rc = NOT_INITED;
    }

    if (SUCCEED(rc)) {
        for (uint32_t i = 0; i < mMemNum; i++) {
            if (NOTNULL(mMem[i].buf)) {
                void *buf = mMem[i].buf;
                mMem[i].buf = NULL;
                mMem[i].size = 0;
                mMem[i].serverfd = -1;
                mMem[i].clientfd = -1;
                rc = mOps->releaseIon(buf);
                if (!SUCCEED(rc)) {
                    LOGF(mModule, "Failed to release memory, will cause "
                        "%d bytes memory leakage, %d", mMem[i].size, rc);
                }
            }
        }
    }

    if (SUCCEED(rc)) {
        free(mMem);
        mMem = NULL;
    }

    return rc;
}

int32_t RequestHandler::shareSingleMem(int32_t fd)
{
    int32_t rc = NO_ERROR;
    char msg[SOCKET_DATA_MAX_LEN];
    int32_t clientfd = -1;

    if (SUCCEED(rc)) {
        sprintf(msg, SOCKET_SERVER_SHARE_STR " %d", getType());
        rc = mOps->sendClientMsg(msg, strlen(msg));
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to send msg %s, %d", msg, rc);
        }
    }

    if (SUCCEED(rc)) {
        msg[0] = '\0';
        rc = mOps->receiveClientMsg(msg, SOCKET_DATA_MAX_LEN);
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
        rc = mOps->sendClientFd(fd);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to send client fd %d, %d", fd, rc);
        }
    }

    if (SUCCEED(rc)) {
        msg[0] = '\0';
        rc = mOps->receiveClientMsg(msg, SOCKET_DATA_MAX_LEN);
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
        rc = convertToClientFd(msg, &clientfd);
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
        rc = mOps->addMemory(mType, clientfd, false);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to add memory to controller, %d", rc);
        }
     }

    return rc;
}

int32_t RequestHandler::convertToClientFd(char *msg, int32_t *clientfd)
{
    int32_t rc = NO_ERROR;

    *clientfd = atoi(msg + strlen(SOCKET_CLIENT_REPLY_FD_STR) + 1);
    if (*clientfd <= 0) {
        LOGE(mModule, "Invalid msg, \"%s\"", msg);
        rc = PARAM_INVALID;
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
    bool locked = false;

    if (!mOps->clientReady()) {
        LOGE(mModule, "Client not ready");
        rc = NOT_READY;
    }

    if (SUCCEED(rc)) {
        locked = true;
        pthread_mutex_lock(&mSocketL);
    }

    if (SUCCEED(rc)) {
        for (uint32_t i = 0; i < mMemNum; i++) {
            rc = shareSingleMem(mMem[i].serverfd);
            if (!SUCCEED(rc)) {
                LOGE(mModule, "Failed to share type %s fd %d",
                    getName(), mMem[i].serverfd);
            }
        }
    }

    if (SUCCEED(rc) || !SUCCEED(rc)) {
        if (locked) {
            pthread_mutex_unlock(&mSocketL);
        }
    }

    return rc;
}

int32_t RequestHandler::onClientReady()
{
    int32_t rc = NO_ERROR;
    int32_t size = 0;

    if (SUCCEED(rc)) {
        rc = mOps->getHeader(mHeader);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to set memory size %d, %d", size, rc);
        }
    }

    if (SUCCEED(rc)) {
        size = getExpectedBufferSize();
        rc = mOps->setMemSize(mType, size);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to set memory %dB to %s, %d", size, )
        }
    }

    if (!mMemShared) {
        rc = allocMemAndShare();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to share mem with client, %d", rc);
        }
    }

    return rc;
}

int32_t RequestHandler::allocMemAndShare()
{
    int32_t rc = NO_ERROR;

    if (SUCCEED(rc)) {
        rc = allocMem();
        if (!SUCCEED(rc)) {
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

    return rc;
}

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
    mOps(ops),
    mMemShared(false),
    mMemNum(memNum),
    mMem(NULL),
    mRunOnce(NULL),
    mAbortOnce(false)
{
    ASSERT_LOG(mModule, NOTNULL(ops), "Ops shouldn't be NULL");
    ASSERT_LOG(mModule, memNum != 0,  "Mem num shoudn't be 0");
    if (ISNULL(name)) {
        name = "generic request handler";
    }
}

RequestHandler::~RequestHandler()
{
    if (mConstructed) {
        destruct();
    }
}

int32_t RequestHandler::enqueue(void *buf, int32_t size)
{
    int32_t rc = NO_ERROR;
    MemoryInfo *mem = NULL;
    bool locked = false;

    if (SUCCEED(rc)) {
        rc = getFirstFreshMemLock(&mem);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to find latest memory, %d", rc);
        }
        if (ISNULL(mem)) {
            LOGD(mModule, "No available client memory to send user buf");
            rc = NO_MEMORY;
        } else {
            locked = true;
        }
    }

    if (!SUCCEED(rc)) {
        rc = mBufMgr.enqueue(buf, size);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to enqueue buf, %d", rc);
        } else {
            rc = NO_MEMORY;
        }
    }

    if (SUCCEED(rc)) {
        rc = copyToUserBuf(mem, buf, size);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to copy to uer buffer, %d", rc);
        }
    }

    if (SUCCEED(rc) || !SUCCEED(rc)) {
        if (locked) {
            rc = mOps->unlockMemory(mType, mem->clientfd);
            if (!SUCCEED(rc)) {
                LOGE(mModule, "Failed to unlock memory %d, %d",
                    mem->clientfd, rc);
            }
        }
    }

    return RETURNIGNORE(rc, NO_MEMORY);
}

int32_t RequestHandler::dequeue(void **buf, int32_t *size)
{
    return mBufMgr.dequeue(buf, size);
}

int32_t RequestHandler::wait()
{
    int32_t rc = NO_ERROR;

    if (NOTNULL(mRunOnce)) {
        LOGE(mModule, "Run once thread already started.");
        rc = ALREADY_EXISTS;
    }

    if (SUCCEED(rc)) {
        mRunOnce = new RunOnce();
        if (ISNULL(mRunOnce)) {
            LOGE(mModule, "Failed to new run once thread, %d", rc);
            rc = NO_MEMORY;
        }
    }

    if (SUCCEED(rc)) {
        // wait for client to return the result of requests
        rc = mRunOnce->run(this, NULL, NULL);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to run once thread, %d", rc);
        }
    }

    return rc;
}

int32_t RequestHandler::abort()
{
    int32_t rc = NO_ERROR;
    int32_t final = rc;

    if (ISNULL(mRunOnce)) {
        LOGE(mModule, "Run once thread alreay exited.");
        rc = NOT_EXIST;
    }

    if (SUCCEED(rc)) {
        if (NOTNULL(mRunOnce)) {
            rc = mRunOnce->exit();
            if (!SUCCEED(rc)) {
                LOGE(mModule, "Failed to abort to once thread, %d", rc);
                final |= rc;
                rc = NO_ERROR;
            }
        }
    }

    if (SUCCEED(rc)) {
        if (NOTNULL(mRunOnce)) {
            delete mRunOnce;
            mRunOnce = NULL;
        }
    }

    return final;
}

int32_t RequestHandler::getExpectedBufferSize()
{
    return getHeaderSize() + getDataSize();
}

int32_t RequestHandler::processClientUpdate()
{
    int32_t rc = NO_ERROR;
    MemoryInfo *mem = NULL;
    bool locked = false;

    if (SUCCEED(rc)) {
        rc = getFirstFreshMemLock(&mem);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to find latest memory, %d", rc);
        } else if (ISNULL(mem)) {
            LOGI(mModule, "No latest memory found, skip...");
            rc = NO_MEMORY;
        } else {
            locked = true;
        }
    }

    if (SUCCEED(rc)) {
        rc = copyToUserBuf(mem);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to copy memory to user buf");
        }
    }

    if (SUCCEED(rc) || !SUCCEED(rc)) {
        if (locked) {
            rc = mOps->unlockMemory(mType, mem->clientfd);
            if (!SUCCEED(rc)) {
                LOGE(mModule, "Failed to unlock memory %d, %d",
                    mem->clientfd, rc);
            }
        }
    }

    return RETURNIGNORE(rc, NO_MEMORY);
}

int32_t RequestHandler::getFirstFreshMemLock(
    RequestHandler::MemoryInfo **mem)
{
    int32_t rc = NO_ERROR;
    int32_t fd = -1;
    int32_t index = -1;

    if (ISNULL(mMem)) {
        LOGD(mModule, "Memory not allocated.");
        rc = NOT_INITED;
    }

    if (SUCCEED(rc)) {
        rc = mOps->getFirstFreshMemLock(mType, &fd);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to get first fresh memory, %d", rc);
        }
        if (fd == -1) {
            LOGE(mModule, "Invalid fd %d", fd);
            rc = UNKNOWN_ERROR;
        }
    }

    if (SUCCEED(rc)) {
        for (uint32_t i = 0; i < mMemNum; i++) {
            if (mMem[i].clientfd == fd) {
                index = i;
                break;
            }
        }
    }

    if (index == -1) {
        LOGD(mModule, "Failed to find available memory from client");
        *mem = NULL;
    } else {
        *mem = &mMem[index];
    }

    return rc;
}

int32_t RequestHandler::copyToUserBuf(
    RequestHandler::MemoryInfo *mem, void *userbuf, int32_t usersize)
{
    int32_t rc = NO_ERROR;
    void *buf = userbuf;
    int32_t datSize = 0;
    int32_t bufSize = usersize;
    int32_t privateSize = 0;

    if (ISNULL(mem)) {
        LOGE(mModule, "Memory not available.");
        rc = PARAM_INVALID;
    }

    if (SUCCEED(rc)) {
        datSize = getExpectedBufferSize();
        if (datSize == 0) {
            LOGE(mModule, "Invalid data size");
            rc = UNKNOWN_ERROR;
        }
    }

    if (SUCCEED(rc)) {
        if (ISNULL(buf)) {
            rc = dequeue(&buf, &bufSize);
            if (!SUCCEED(rc)) {
                LOGE(mModule, "Failed to dequeue user buf, %d", rc);
            }
        }
    }

    if (SUCCEED(rc)) {
        if (ISNULL(buf) || bufSize == 0) {
            LOGI(mModule, "No available user buf, frame abandon...");
            rc = NO_MEMORY;
        }
    }

    if (SUCCEED(rc)) {
        if (datSize > bufSize) {
            LOGE(mModule, "Data size bigger than user buffer size");
            rc = UNKNOWN_ERROR;
        }
        if (datSize > mem->size) {
            LOGE(mModule, "Data size bigger than client buffer size");
            rc = UNKNOWN_ERROR;
        }
    }

    if (SUCCEED(rc)) {
        int32_t copied = 0;
        rc = copyHeader(buf, mem->buf, &copied);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to copy private data");
        }
    }

    if (SUCCEED(rc)) {
        privateSize = getHeaderSize();
        if (privateSize == 0) {
            LOGD(mModule, "No private data");
        }
    }

    if (SUCCEED(rc)) {
        copyData((int8_t *)buf + privateSize,
            (int8_t *)mem->buf + privateSize,
            getDataSize() < mem->size ?
                getDataSize() : mem->size);
    }

    if (SUCCEED(rc)) {
        rc = mOps->setMemStatus(mType, mem->clientfd, false);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to update memory status, %d",
                mem->clientfd, rc);
        }
    }

    return rc;
}

int32_t RequestHandler::copyData(void *dst, void *src, int32_t len)
{
    memcpy(dst, src, len);
    return NO_ERROR;
}

int32_t RequestHandler::runOnceFunc(void * /*in*/, void * /*out*/)
{
    int32_t rc = NO_ERROR;
    Semaphore *sem = NULL;

    if (SUCCEED(rc)) {
        rc = mOps->waitClientSem(mType, &sem);
        if (!SUCCEED(rc) || ISNULL(sem)) {
            LOGE(mModule, "Failed to get client semaphore %p, %d", sem, rc);
            rc = UNKNOWN_ERROR;
        }
    }

    if (SUCCEED(rc)) {
        do {
            sem->wait();

            if (mAbortOnce) {
                break;
            }

            rc = processClientUpdate();
            if (!SUCCEED(rc)) {
                LOGE(mModule, "Failed to process client update, %d", rc);
            }
        } while (true);
    }

    return rc;
}

int32_t RequestHandler::onOnceFuncFinished(int32_t /*rc*/)
{
    // will be here when request handler been destructed
    return NO_ERROR;
}

int32_t RequestHandler::abortOnceFunc()
{
    int32_t rc = NO_ERROR;
    Semaphore *sem = NULL;

    if (SUCCEED(rc)) {
        rc = mOps->waitClientSem(mType, &sem);
        if (!SUCCEED(rc) || ISNULL(sem)) {
            LOGE(mModule, "Failed to get client semaphore %p, %d", sem, rc);
            rc = UNKNOWN_ERROR;
        }
    }

    if (SUCCEED(rc)) {
        mAbortOnce = true;
        sem->signal();
    }

    return NO_ERROR;
}

int32_t RequestHandler::RunOnce::run(RunOnceFunc *func, void *in, void *out)
{
    int32_t rc = NO_ERROR;

    if (ISNULL(func)) {
        LOGE(MODULE_RUN_ONCE_THREAD, "func can't be null");
        rc = PARAM_INVALID;
    }

    if (SUCCEED(rc)) {
        rc = RunOnceThread::run(func, in, out);
        if (!SUCCEED(rc)) {
            LOGE(MODULE_RUN_ONCE_THREAD, "Failed to start thread");
        }
    }

    return rc;
}

int32_t RequestHandler::RunOnce::exit()
{
    return RunOnceThread::exit();
}

bool RequestHandler::RunOnce::isRuning()
{
    return RunOnceThread::isRuning();
}

};

