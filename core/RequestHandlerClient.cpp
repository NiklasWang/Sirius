#include "RequestHandlerClient.h"

namespace sirius {

int32_t RequestHandlerClient::construct()
{
    int32_t rc = NO_ERROR;

    if (mConstructed) {
        rc = ALREADY_INITED;
    }

    if (ISNULL(mMemMap)) {
        int32_t num = REQUEST_TYPE_MAX_INVALID *
            REQUEST_HANDLER_MAX_MEMORY_NUM;
        mMemMap = (MemoryMap *)malloc(sizeof(MemoryMap) * num);
        if (ISNULL(mMemMap)) {
            LOGE(mModule, "Failed to alloc %d bytes.",
                sizeof(MemoryMap) * num);
        } else {
            for (int32_t i = 0; i < num; i++) {
                mMemMap[i].fd = -1;
                mMemMap[i].mem = NULL;
                mMemMap[i].size = 0;
            }
        }
    }

    if (SUCCEED(rc)) {
        rc = mBufMgr.init();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to init ion buf mgr, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        pthread_mutex_init(&mSocketL, NULL);
        rc = mSC.construct();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to construct socket state machine, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        // try to connect to server
        rc = mRunOnce.run(this, NULL, NULL);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to run once thread, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        mConstructed = true;
        LOGD(mModule, "Sirius client core constructed");
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
        mCtlReady = false;
        mServerConnected = false;
    }

    if (SUCCEED(rc)) {
        mSC.cancelWaitMsg();
        rc = mSC.destruct();
        if (!SUCCEED(rc)) {
            final |= rc;
            LOGE(mModule, "Failed to destruct socket state machine, %d", rc);
            rc = NO_ERROR;
        }
        pthread_mutex_destroy(&mSocketL);
    }

    if (SUCCEED(rc)) {
        rc = mNotifier.deinit();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to init notifier, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = abortOnceFunc();
        if (!SUCCEED(rc)) {
            final |= rc;
            LOGE(mModule, "Failed to abort run once thread");
            rc = NO_ERROR;
        }
    }

    if (SUCCEED(rc)) {
        rc = mRunOnce.exit();
        if (!SUCCEED(rc)) {
            final |= rc;
            LOGE(mModule, "Failed to exit once thread, %d", rc);
            rc = NO_ERROR;
        }
    }

    if (SUCCEED(rc)) {
        rc = releaseAllMems();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to release all mems, %d", rc);
        }
        rc = NO_ERROR;
    }


    if (SUCCEED(rc)) {
        if (NOTNULL(mMemMap)) {
            free(mMemMap);
            mMemMap = NULL;
        }
    }

    if (SUCCEED(rc)) {
        mBufMgr.clear_all();
        rc = mBufMgr.deinit();
        if (!SUCCEED(rc)) {
            final |= rc;
            LOGE(mModule, "Failed to deinit ion buf mgr, %d", rc);
            rc = NO_ERROR;
        }
    }

    if (!SUCCEED(final)) {
        LOGE(mModule, "Sirius client core destructed with error %d", final);
    } else {
        LOGD(mModule, "Sirius client core destructed");
    }

    return rc;

}

int32_t RequestHandlerClient::findMemoryMap(
    RequestType type, int32_t fd, void **mem, int32_t *size)
{
    int32_t rc = NO_ERROR;
    void *result = NULL;
    int32_t maxSize = -1;

    MemoryMap *mems = (MemoryMap *)(
        (int8_t *)mMemMap + sizeof(MemoryMap) * type);

    for (int32_t i = 0; i < REQUEST_HANDLER_MAX_MEMORY_NUM; i++) {
        if (mems[i].fd == fd &&
            mems[i].fd != -1) {
            result = mems[i].mem;
            maxSize = mems[i].size;
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
    int32_t num = REQUEST_TYPE_MAX_INVALID *
        REQUEST_HANDLER_MAX_MEMORY_NUM;

    for (int32_t i = 0; i < num; i++) {
        if (mMemMap[i].mem != NULL) {
            mBufMgr.release(mMemMap[i].mem);
            mMemMap[i].fd = -1;
            mMemMap[i].mem = NULL;
            mMemMap[i].size = 0;
        }
    }

    return rc;
}

bool RequestHandlerClient::requested(RequestType type)
{
    bool result = false;

    if (mCtlReady && mCtl.requested(type)) {
        result = true;
    }

    return result;
}

int32_t RequestHandlerClient::onDataReady(
    ClientRequestIntf *intf, RequestType type)
{
    int32_t rc = NO_ERROR;
    int32_t fd = -1;
    void *mem = NULL;
    bool locked = false;
    int32_t maxSize = -1;

    if (!mCtlReady) {
        rc = NOT_READY;
    }

    if (SUCCEED(rc)) {
        if (!requested(type)) {
            LOGI(mModule, "%d not requested, abandon this time.", type);
            rc = NOT_REQUIRED;
        }
    }

    if (SUCCEED(rc)) {
        rc = mCtl.getUsedMem(type, &fd);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to get used memory, %d", rc);
        } else if (fd == -1) {
            LOGE(mModule, "Sirius server memory not ready, skip this frame");
            rc = NO_MEMORY;
        } else {
            locked = true;
        }
    }

    if (SUCCEED(rc)) {
        rc = findMemoryMap(type, fd, &mem, &maxSize);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to find memory map, %d", rc);
        }
        if (ISNULL(mem)) {
            LOGE(mModule, "Didn't find memory in map");
            rc = NOT_FOUND;
        }
    }

    if (SUCCEED(rc)) {
        rc = copyToServer(intf, mem, maxSize);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to copy to server, %d", rc);
        }
    }

    if (SUCCEED(rc) || !SUCCEED(rc)) {
        if (locked) {
            rc = mCtl.unlockMemory(type, fd);
            if (!SUCCEED(rc)) {
                LOGE(mModule, "Failed to unlock memory, %d", rc);
            }
        }
    }

    if (SUCCEED(rc)) {
        rc = mCtl.setMemStatus(type, fd, true);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to renew memory status, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = mNotifier.notify(type, fd);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to notify fd %d update, %d", fd, rc);
        }
    }

    return rc;
}

int32_t RequestHandlerClient::copyToServer(ClientRequestIntf *intf,
    void *dst, int32_t maxSize)
{
    int32_t rc = NO_ERROR;
    int32_t sizeOfP = -1;

    if (SUCCEED(rc)) {
        sizeOfP = intf->sizeOfHeader();
        if (sizeOfP < 0) {
            LOGE(mModule, "Failed to get size of private data, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = intf->copyHeader(dst, maxSize);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to copy private data, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = intf->copyData((char *)dst + sizeOfP, maxSize - sizeOfP);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to copy data, %d", rc);
        }
    }

    return rc;
}

int32_t RequestHandlerClient::runOnceFunc(void * /*in*/, void * /*out*/)
{
    int32_t rc = NO_ERROR;
    int32_t fd = -1;
    int32_t size = 0;
    void *buf = NULL;

    do {
        rc = mSC.connectServer();
        if (!SUCCEED(rc)) {
            usleep(CLIENT_CONNECT_RETRY_TIME * 1000);
        }
    } while(!SUCCEED(rc) && !mAbortOnce);

    if (SUCCEED(rc)) {
        rc = mNotifier.init();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to init notifier, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = mSC.sendMsg(SOCKET_CLIENT_GREETING_STR,
            strlen(SOCKET_CLIENT_GREETING_STR));
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to send msg \"%s\" to server, %d",
                SOCKET_CLIENT_GREETING_STR, rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = mSC.receiveFd(&fd);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to receive fd from server, %d", rc);
        }
        if (fd == -1) {
            LOGE(mModule, "Invalid fd received from server");
            rc = UNKNOWN_ERROR;
        }
    }

    if (SUCCEED(rc)) {
        rc = mSC.sendMsg(SOCKET_CLIENT_REPLY_STR,
            strlen(SOCKET_CLIENT_REPLY_STR));
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to send msg \"%s\" to server, %d",
                SOCKET_CLIENT_REPLY_STR, rc);
        }
    }

    if (SUCCEED(rc)) {
        size = mCtl.getTotoalSize();
        if (size <= 0) {
            LOGE(mModule, "Invalid control blcok size %d", size);
            rc = PARAM_INVALID;
        }
    }

    if (SUCCEED(rc)) {
        rc = mBufMgr.import(&buf, fd, size);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to import %d bytes memory %d, %d",
                size, fd, rc);
        }
        if (ISNULL(buf)) {
            LOGE(mModule, "Invalid import memory result.");
            rc = UNKNOWN_ERROR;
        }
    }

    if (SUCCEED(rc)) {
        rc = mCtl.init(buf, size, false);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to set memory to controller, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        mCtlReady = true;
    }

    if (SUCCEED(rc)) {
        // run in loop inside until exits
        rc = waitForPushRequestMemory();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to wait for register request memory, %d", rc);
        }
    }

    if (SUCCEED(rc) || !SUCCEED(rc)) {
        if (mAbortOnce) {
            LOGD(mModule, "Abort run one func");
            rc = USER_ABORTED;
        }
    }

    return rc;
}

int32_t RequestHandlerClient::acceptSingleRequestMemory()
{
    int32_t rc = NO_ERROR;
    char msg[SOCKET_DATA_MAX_LEN];
    RequestType type = REQUEST_TYPE_MAX_INVALID;
    int32_t clientfd = -1;
    void *buf = NULL;
    int32_t size = -1;

    pthread_mutex_lock(&mSocketL);

    if (SUCCEED(rc) && !mAbortOnce) {
        msg[0] = '\0';
        rc = mSC.receiveMsg(msg, SOCKET_DATA_MAX_LEN);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to recvmsg from server, %d", rc);
        }
    }

    if (SUCCEED(rc) && !mAbortOnce) {
        if (strncmp(msg, SOCKET_SERVER_SHARE_STR,
            strlen(SOCKET_SERVER_SHARE_STR))) {
            LOGE(mModule, "Unknown msg from server \"%s\", %d", msg, rc);
            rc = INVALID_FORMAT;
        }
    }

    if (SUCCEED(rc) && !mAbortOnce) {
        rc = convertToRequestType(msg, &type);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to convert to request type, %d", rc);
        }
        if (type < 0 || type >= REQUEST_TYPE_MAX_INVALID) {
            LOGE(mModule, "Invalid requtest type %d received", type);
            rc = PARAM_INVALID;
        }
    }

    if (SUCCEED(rc) && !mAbortOnce) {
        strcpy(msg, SOCKET_CLIENT_REPLY_SHARE_STR);
        rc = mSC.sendMsg(msg, strlen(msg));
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to send msg to server, %d", rc);
        }
    }

    if (SUCCEED(rc) && !mAbortOnce) {
        rc = mSC.receiveFd(&clientfd);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to receive fd from server, %d", rc);
        }
        if (clientfd == -1) {
            LOGE(mModule, "Invalid fd -1 from server");
            rc = INVALID_FORMAT;
        }
    }

    if (SUCCEED(rc) && !mAbortOnce) {
        sprintf(msg, SOCKET_CLIENT_REPLY_FD_STR " %d", clientfd);
        rc = mSC.sendMsg(msg, strlen(msg));
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to send msg to server, %d", rc);
        }
    }

    pthread_mutex_unlock(&mSocketL);

    if (SUCCEED(rc) && !mAbortOnce) {
        rc = mCtl.getMemSize(type, &size);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to send msg to server, %d", rc);
        }
        if (size <= 0) {
            LOGE(mModule, "Failed to get memory size");
            rc = UNKNOWN_ERROR;
        }
    }

    if (SUCCEED(rc) && !mAbortOnce) {
        rc = mBufMgr.import(&buf, clientfd, size);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to import memory for fd %d, %d",
                clientfd, rc);
        } else {
            LOGD(mModule, "Request type %d memory fd %d mapped %p",
                type, clientfd, buf);
        }
        if (ISNULL(buf)) {
            LOGE(mModule, "Buf is NULL and that's invalid");
            rc = UNKNOWN_ERROR;
        }
    }

    if (SUCCEED(rc) && !mAbortOnce) {
        rc = addMemoryMap(type, buf, clientfd, size);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to record memory map, %d", rc);
            mBufMgr.release(buf);
        }
    }

    if (SUCCEED(rc) || !SUCCEED(rc)) {
        if (mAbortOnce) {
            LOGD(mModule, "Abort this run once func");
            rc = USER_ABORTED;
        }
    }

    return rc;
}

int32_t RequestHandlerClient::addMemoryMap(RequestType type,
    void *mem, int32_t fd, int32_t size)
{
    int32_t rc = NOT_FOUND;

    MemoryMap *mems = (MemoryMap *)(
        (int8_t *)mMemMap + sizeof(MemoryMap) * type);

    for (int32_t i = 0; i < REQUEST_HANDLER_MAX_MEMORY_NUM; i++) {
        if (mems[i].fd == -1) {
            mems[i].mem = mem;
            mems[i].fd  = fd;
            mems[i].size = size;
            rc = NO_ERROR;
            break;
        }
    }

    return rc;
}

int32_t RequestHandlerClient::waitForPushRequestMemory()
{
    int32_t rc = NO_ERROR;

    while (!mAbortOnce) {
        rc = acceptSingleRequestMemory();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to accept one request memory");
        }
    }

    return rc;
}

int32_t RequestHandlerClient::onOnceFuncFinished(int32_t /*rc*/)
{
    return NO_ERROR;
}

int32_t RequestHandlerClient::abortOnceFunc()
{
    mAbortOnce = true;
    mSC.cancelWaitMsg();
    return NO_ERROR;
}

int32_t RequestHandlerClient::RunOnce::run(RunOnceFunc *func, void *in, void *out)
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

int32_t RequestHandlerClient::RunOnce::exit()
{
    return RunOnceThread::exit();
}

bool RequestHandlerClient::RunOnce::isRuning()
{
    return RunOnceThread::isRuning();
}

int32_t RequestHandlerClient::ReadyNotifier::init()
{
    int32_t rc = NO_ERROR;

    mExit = false;

    if (SUCCEED(rc)) {
        rc = construct();
        if (!SUCCEED(rc)) {
            LOGE(MODULE_SIRIUS_CLIENT_CORE, "Failed to "
                "construct ReadyNotifier", rc);
        }
    }

    if (SUCCEED(rc)) {
        if (!mExit) {
            do {
                rc = connectServer();
                if (!SUCCEED(rc)) {
                    usleep(CLIENT_CONNECT_NOTIFIER_TIME * 1000);
                }
            } while(!SUCCEED(rc) && !mExit);
        }
    }

    if (SUCCEED(rc) || !SUCCEED(rc)) {
        if (mExit) {
            LOGD(MODULE_SIRIUS_CLIENT_CORE, "Abort run one func");
            rc = USER_ABORTED;
        }
    }

    return rc;
}

int32_t RequestHandlerClient::ReadyNotifier::deinit()
{
    mExit = true;
    return destruct();
}

int32_t RequestHandlerClient::ReadyNotifier::notify(
    RequestType type, int32_t clientfd)
{
    char msg[SOCKET_DATA_MAX_LEN];
    sprintf(msg, SOCKET_CLIENT_NOTIFIER_STR " %d %d", type, clientfd);
    return sendMsg(msg, strlen(msg));
}

RequestHandlerClient::ReadyNotifier::ReadyNotifier() :
    mExit(false)
{
}

RequestHandlerClient::RequestHandlerClient() :
    mConstructed(false),
    mModule(MODULE_SIRIUS_CLIENT_CORE),
    mServerConnected(false),
    mCtlReady(false),
    mAbortOnce(false),
    mMemMap(NULL)
{}

RequestHandlerClient::~RequestHandlerClient()
{
}

};

