#include "SiriusCore.h"
#include "EventRequestServer.h"

namespace sirius {

int32_t SiriusCore::construct()
{
    int32_t rc = NO_ERROR;
    int32_t size = 0;

    if (mConstructed) {
        rc = ALREADY_INITED;
    }

    if (SUCCEED(rc)) {
        rc = mIon.init();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to init ion buf mgr, %d", rc);
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
        rc = mIon.allocate(&mCtlMem, size, &mCtlFd);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to alloc %dB ion buf", size);
        }
    }

    if (SUCCEED(rc)) {
        rc = mCtl.setMemory(mCtlMem, size, true);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to set memory to control mgr, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = mSS.construct();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to construct server state machine, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = mCb.construct();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to construct callback thread, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        // start socket server and wait for client
        rc = mRunOnce->run(this, NULL, NULL);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to run once thread, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        mConstructed = true;
        LOGD(mModule, "Sirius core constructed");
    }

    return rc;
}

int32_t SiriusCore::runOnceFunc(void * /*in*/, void * /*out*/)
{
    int32_t rc = NO_ERROR;

    if (SUCCEED(rc)) {
        rc = mSS.startServer();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to start socket server");
        }
    }

    if (SUCCEED(rc)) {
        rc = mSS.waitForConnect();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to wait for client connection");
        }
    }

    if (SUCCEED(rc)) {
        if (!mSS.connected()) {
            LOGD(mModule, "Client not connected, exit.");
            rc = NOT_EXIST;
        }
    }

    if (SUCCEED(rc)) { // rc is TIMEDOUT if cancelled to wait for connection
        mSocketMsg[0] = '\0';
        rc = mSS.receiveMsg(mSocketMsg, sizeof(mSocketMsg));
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to receive msg from socket sm, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        if (strcmp(mSocketMsg, SOCKET_CLIENT_GREETING_STR)) {
            mSocketMsg[sizeof(mSocketMsg) - 1] = '\0';
            LOGE(mModule, "Unknown msg received, \"%s\"", mSocketMsg);
            rc = NOT_READY;
        }
    }

    if (SUCCEED(rc)) {
        rc = mSS.sendFd(mCtlFd);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to send fd %d to client, %d", mCtlFd, rc);
        }
    }

    if (SUCCEED(rc)) {
        mSocketMsg[0] = '\0';
        rc = mSS.receiveMsg(mSocketMsg, sizeof(mSocketMsg));
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to receive msg from socket sm, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        if (strcmp(mSocketMsg, SOCKET_CLIENT_REPLY_STR)) {
            mSocketMsg[sizeof(mSocketMsg) - 1] = '\0';
            LOGE(mModule, "Unknown msg received, \"%s\"", mSocketMsg);
            rc = NOT_READY;
        }
    }

    if (SUCCEED(rc)) {
        rc = enableCachedRequests();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to enable cached requests, %d", rc);
        }
    }

    do {
        int32_t clientfd = -1;
        RequestType type = REQUEST_TYPE_MAX_INVALID;
        RESETRESULT(rc);

        if (SUCCEED(rc)) {
            rc = mSS.waitForConnect(&clientfd);
            if (!SUCCEED(rc)) {
                LOGE(mModule, "Failed to wait for client connection");
            }
            if (rc == USER_ABORTED) {
                LOGI(mModule, "Stop wait connect, aborted.");
                break;
            }
        }

        if (SUCCEED(rc)) {
            mSocketMsg[0] = '\0';
            rc = mSS.receiveMsg(clientfd, mSocketMsg, sizeof(mSocketMsg));
            if (!SUCCEED(rc)) {
                LOGE(mModule, "Failed to receive msg, %d", rc);
            }
        }

        if (SUCCEED(rc)) {
            rc = convertToRequestType(mSocketMsg, &type);
            if (!SUCCEED(rc) || type == REQUEST_TYPE_MAX_INVALID) {
                LOGE(mModule, "Invalid socket msg, %s", mSocketMsg);
            }
        }

        if (SUCCEED(rc)) {
            rc = mRequests[type]->setSocketFd(clientfd);
            if (!SUCCEED(rc)) {
                LOGE(mModule, "Failed to set socket fd %d to %s",
                    clientfd, mRequests[type]->getName());
            }
        }
    
        if (SUCCEED(rc)) {
            rc = mRequests[type]->onClientReady();
            if (!SUCCEED(rc)) {
                LOGE(mModule, "Failed to notify client connected to %s",
                    rc, mRequests[type]->getName());
            }
        }
    } while(rc != USER_ABORTED);

    return rc;
}

int32_t SiriusCore::convertToRequestType(
    char *msg, RequestType *type)
{
    int32_t rc = NO_ERROR;
    int32_t value = atoi(msg + strlen(SOCKET_CLIENT_CONNECT_TYPE) + 1);

    *type = REQUEST_TYPE_MAX_INVALID;
    if (value < 0) {
        LOGE(mModule, "Invalid msg, \"%s\"", msg);
        rc = PARAM_INVALID;
    } else {
        *type = static_cast<RequestType>(value);
    }

    return rc;
}

int32_t SiriusCore::enableCachedRequests()
{
    int32_t rc = NO_ERROR;

    for (int32_t i = 0; i < REQUEST_TYPE_MAX_INVALID; i++) {
        if (mCachedRequest[i]) {
            LOGD(mModule, "Enable cached request %d", i);
            rc = request(static_cast<RequestType>(i));
            if (!SUCCEED(rc)) {
                LOGE(mModule, "Failed to create cached request %d", rc);
            } else {
                mCachedRequest[i] = false;
            }
        }
    }


    return rc;
}

int32_t SiriusCore::onOnceFuncFinished(int32_t /*rc*/)
{
    return NO_ERROR;
}

int32_t SiriusCore::abortOnceFunc()
{
    int32_t rc = NO_ERROR;

    if (!mSS.connected()) {
        rc = mSS.cancelWaitConnect();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to cancel wait client");
        } else {
            mExit = true;
        }
    }

    return rc;
}

int32_t SiriusCore::destruct()
{
    int32_t rc = NO_ERROR;
    int32_t final = NO_ERROR;

    if (!mConstructed) {
        rc = NOT_INITED;
    } else {
        mConstructed = false;
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
        for (int32_t i = 0; i < REQUEST_TYPE_MAX_INVALID; i++) {
            rc = mCtl->setRequest(gRequestTypeMap[i], false);
            if (!SUCCEED(rc)) {
                LOGE(mModule, "Failed to cancel request %d", i);
            }
        }
    }
    
    if (SUCCEED(rc)) {
        for (int32_t i = 0; i < REQUEST_TYPE_MAX_INVALID; i++) {
            if (NOTNULL(mRequests[i])) {
                rc = mRequests[i]->abort();
                if (!SUCCEED(rc)) {
                    final |= rc;
                    LOGE(mModule, "Failed to abort request handler %s",
                        mRequests[i]->getName());
                    rc = NO_ERROR;
                }
                rc = mRequests[i]->destruct();
                if (!SUCCEED(rc)) {
                    final |= rc;
                    LOGE(mModule, "Failed to destruct request handler %s",
                        mRequests[i]->getName());
                    rc = NO_ERROR;
                }
                SECURE_DELETE(mRequests[i]);
            }
        }
    }

    if (SUCCEED(rc)) {
        rc = mSS.destruct();
        if (!SUCCEED(rc)) {
            final |= rc;
            LOGE(mModule, "Failed to destruct socket state machine, %d", rc);
            rc = NO_ERROR;
        }
    }

    if (SUCCEED(rc)) {
        rc = mCb.destruct();
        if (!SUCCEED(rc)) {
            final |= rc;
            LOGE(mModule, "Failed to destruct callback thread, %d", rc);
            rc = NO_ERROR;
        }
    }

    if (SUCCEED(rc)) {
        if (NOTNULL(mRunOnce)) {
            rc = mRunOnce->exit();
            if (!SUCCEED(rc)) {
                final |= rc;
                LOGE(mModule, "Failed to exit rcun once thread, %d", rc);
                rc = NO_ERROR;
            }
            SECURE_DELETE(mRunOnce);
        }
    }

    if (SUCCEED(rc)) {
        if (NOTNULL(mCtlMem)) {
            rc = mIon.release(mCtlMem);
            if (!SUCCEED(rc)) {
                final |= rc;
                LOGE(mModule, "Failed to release ion buf, %d", rc);
                rc = NO_ERROR;
            }
        }
    }

    if (SUCCEED(rc)) {
        if (NOTNULL(mCtlMem)) {
            // control memory freed by munmap() call
            mCtlMem = NULL;
        }
    }

    if (SUCCEED(rc)) {
        mIon.clear_all();
        rc = mIon.deinit();
        if (!SUCCEED(rc)) {
            final |= rc;
            LOGE(mModule, "Failed to deinit ion buf mgr, %d", rc);
            rc = NO_ERROR;
        }
    }

    if (!SUCCEED(final)) {
        LOGE(mModule, "Sirius core destructed with error %d", final);
    } else {
        LOGD(mModule, "Sirius core destructed");
    }

    return rc;
}

RequestType getRequestType(RequestType type)
{
    return (type < 0 || type > REQUEST_TYPE_MAX_INVALID) ?
        REQUEST_TYPE_MAX_INVALID : type;
}

int32_t SiriusCore::createRequestHandler(RequestType type, bool wait)
{
    int32_t rc = NO_ERROR;
    RequestHandler *requestHandler = NULL;

    if (SUCCEED(rc)) {
        if (NOTNULL(mRequests[type])) {
            LOGI(mModule, "%s alreay requested",
                mRequests[type]->getName());
            rc = ALREADY_EXISTS;
        }
    }

    if (SUCCEED(rc)) {
        if (type == EXTENDED_EVENT && ISNULL(mEvtSvr)) {
            mEvtSvr = new EventRequestServer(this, mServerSocket);
            if (ISNULL(mEvtSvr)) {
                LOGE(mModule, "No memory to new EventRequestServer");
                rc = NO_MEMORY;
            }
        }
    }

    if (SUCCEED(rc)) {
        if (type == EXTENDED_EVENT) {
            rc = mEvtSvr->construct();
            if (!SUCCEED(rc)) {
                LOGE(mModule, "Failed to construct evt server");
            } else {
                rc = JUMP_DONE;
            }
        }
    }

    if (SUCCEED(rc)) {
        requestHandler = createRequestHandler(type);
        if (ISNULL(requestHandler)) {
            LOGE(mModule, "Failed to create request handler");
            rc = NO_MEMORY;
        }
    }

    if (SUCCEED(rc)) {
        rc = requestHandler->construct();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to const request handler, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        mRequests[type] = requestHandler;
    }

    if (SUCCEEDIGNORE(rc, ALREADY_EXISTS)) {
        if (wait) {
            rc = mRequests[type]->wait();
            if (!SUCCEED(rc)) {
                LOGE(mModule, "Failed to enter wait status, %d", rc);
            }
        }
    }

    if (SUCCEEDIGNORE(rc, ALREADY_EXISTS)) {
        if (wait) {
            rc = setRequestedMark(type, true);
            if (!SUCCEED(rc)) {
                LOGE(mModule, "Failed to set enable request %d "
                    "for client, %d", type, rc);
            }
        }
    }

    if (!SUCCEED(rc)) {
        if (NOTNULL(requestHandler)) {
            delete requestHandler;
            requestHandler = NULL;
            mRequests[type] = NULL;
        }
    }

    return RETURNIGNORE(rc, JUMP_DONE);
}

int32_t SiriusCore::request(RequestType type)
{
    int32_t rc = NO_ERROR;

    if (SUCCEED(rc)) {
        type = getRequestType(type);
        if (type == REQUEST_TYPE_MAX_INVALID) {
            LOGE(mModule, "Invalid request type %d", type);
            rc = PARAM_INVALID;
        }
    }

    if (SUCCEED(rc)) {
        if (!clientReady()) {
            if (!clientReady()) {
                mCachedRequest[type] = true;
                LOGD(mModule, "Client not ready, request %d cached.", type);
            }
        }
    }

    if (SUCCEED(rc)) {
        if (clientReady()) {
            rc = createRequestHandler(type, true);
            if (!SUCCEED(rc)) {
                LOGE(mModule, "Failed to create request handler.");
            }
        }
    }

    return rc;
}

int32_t SiriusCore::abortRequest(RequestType type)
{
    int32_t rc = NO_ERROR;
    RequestHandler *requestHandler = NULL;

    if (SUCCEED(rc)) {
        if (!requested(type)) {
            LOGI(mModule, "%d not requested", type);
            rc = NOT_INITED;
        }
    }

    if (SUCCEED(rc)) {
        rc = setRequestedMark(type, false);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to set disable request %d "
                "for client, %d", type, rc);
        }
    }

    if (SUCCEED(rc)) {
        if (type == EXTENDED_EVENT && NOTNULL(mEvtSvr)) {
            rc = mEvtSvr->destruct();
            if (!SUCCEED(rc)) {
                LOGE(mModule, "Failed to destruct evt server");
            }
            SECURE_DELETE(mEvtSvr);
        }
        rc = JUMP_DONE;
    }

    if (SUCCEED(rc)) {
        requestHandler = mRequests[type];
        rc = requestHandler->abort();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to exit request handler %s, %d",
                requestHandler->getName(), rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = requestHandler->destruct();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to destruct request handler %s, %d",
                requestHandler->getName(), rc);
        }
    }

    if (SUCCEED(rc)) {
        delete requestHandler;
        mRequests[type] = NULL;
    }

    return RETURNIGNORE(rc, JUMP_DONE);
}

int32_t SiriusCore::abort(RequestType type)
{
    int32_t rc = NO_ERROR;

    if (SUCCEED(rc)) {
        type = getRequestType(type);
        if (type == REQUEST_TYPE_MAX_INVALID) {
            LOGE(mModule, "Invalid request type %d", type);
            rc = PARAM_INVALID;
        }
    }

    if (SUCCEED(rc)) {
        if (!clientReady()) {
            if (!clientReady()) {
                mCachedRequest[type] = false;
            }
        }
    }

    if (SUCCEED(rc)) {
        if (clientReady()) {
            rc = abortRequest(type);
            if (!SUCCEED(rc)) {
                LOGE(mModule, "Failed to abort request.");
            }
        }
    }

    return rc;
}

bool SiriusCore::requested(RequestType type)
{
    int32_t rc = NO_ERROR;
    bool result = false;

    if (SUCCEED(rc)) {
        type = getRequestType(type);
        if (type == REQUEST_TYPE_MAX_INVALID) {
            LOGE(mModule, "Invalid request type %d", type);
            rc = PARAM_INVALID;
        }
    }

    if (SUCCEED(rc)) {
        if (NOTNULL(mRequests[type])) {
            if (type == mRequests[type]->getType()) {
                result = true;
            } else {
                LOGE(mModule, "Shouldn't be here, expected %d actual %d",
                    type, mRequests[type]->getType());
            }
        }
    }

    return result;
}

int32_t SiriusCore::enqueueBuf(RequestType type, void *buf, int32_t size)
{
    int32_t rc = NO_ERROR;

    if (SUCCEED(rc)) {
        if (ISNULL(buf) || NEGITIVE_NUM(size) || ISZERO(size)) {
            LOGE(mModule, "Invalid buffer %p %d", buf, size);
            rc = PARAM_INVALID;
        }
    }

    if (SUCCEED(rc)) {
        type = getRequestType(type);
        if (type == REQUEST_TYPE_MAX_INVALID) {
            LOGE(mModule, "Invalid request type %d", type);
            rc = PARAM_INVALID;
        }
    }

    if (SUCCEED(rc)) {
        if (!requested(type)) {
            LOGI(mModule, "%d not requested, create handler first", type);
            rc = createRequestHandler(type, false);
            if (!SUCCEED(rc)) {
                LOGE(mModule, "Failed to create request handler, %d", rc);
            }
        }
    }

    if (SUCCEED(rc)) {
        rc = mRequests[type]->enqueue(buf, size);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to enqueue user buffer %d bytes"
                "to request handler, %d", size, rc);
        }
    }

    return rc;
}

int32_t SiriusCore::dequeueBuf(RequestType type, void *buf)
{
    int32_t rc = NO_ERROR;
    void *addr = NULL;
    int32_t size = 0;

    if (SUCCEED(rc)) {
        if (!requested(type)) {
            LOGI(mModule, "%d not requested", type);
            rc = NOT_INITED;
        }
    }

    if (SUCCEED(rc)) {
        rc = mRequests[type]->dequeue(&addr, &size);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to dequeue user buffer, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        void *first = addr;
        while (addr != NULL) {
            if (buf == addr) {
                // Found matched buf
                break;
            }

            if (buf != ALL_BUFFERS) {
                // Insert to tail, and dequeue from head
                rc = enqueueBuf(type, addr, size);
                if (!SUCCEED(rc)) {
                    LOGE(mModule, "Failed to enqueue again, %d", rc);
                    rc = NO_ERROR;
                }
            }

            if (addr == first) {
                // Not found in buffer loop
                rc = NOT_FOUND;
                break;
            }

            if (SUCCEED(rc)) {
                rc = mRequests[type]->dequeue(&addr, &size);
                if (!SUCCEED(rc)) {
                    LOGE(mModule, "Failed to dequeue user buffer, %d", rc);
                }
            }
        }
    }

    return rc;
}

int32_t SiriusCore::setCallback(callback_func func)
{
    return mCb.setCb(func);
}

int32_t SiriusCore::sendCallback(RequestType type, void *data)
{
    return mCb.send(type, data);
}

#define CHECK_CLIENT_STATUS() \
    ({ \
        int32_t __rc = NO_ERROR; \
        if (!clientReady()) { \
            LOGD(mModule, "Client not connected yet"); \
            __rc = NOT_READY; \
        } \
        __rc; \
    })

int32_t SiriusCore::sendClientFd(int32_t fd)
{
    int32_t rc = CHECK_CLIENT_STATUS();
    return SUCCEED(rc) ? mSS.sendFd(fd) : rc;
}

int32_t SiriusCore::sendClientMsg(const char *data, int32_t len)
{
    int32_t rc = CHECK_CLIENT_STATUS();
    return SUCCEED(rc) ? mSS.sendMsg(data, len) : rc;
}

int32_t SiriusCore::receiveClientMsg(char *data, int32_t maxlen)
{
    int32_t rc = CHECK_CLIENT_STATUS();
    return SUCCEED(rc) ? mSS.receiveMsg(data, maxlen) : rc;
}

int32_t SiriusCore::allocateIon(void **buf, int32_t len, int32_t *fd)
{
    return mIon.allocate(buf, len, fd);
}

int32_t SiriusCore::releaseIon(void *buf)
{
    return mIon.release(buf);
}

int32_t SiriusCore::getFirstFreshMemLock(RequestType type, int32_t *fd)
{
    return mCtl.getFirstFreshMemLock(type, fd);
}

int32_t SiriusCore::getUsedMemLock(RequestType type, int32_t *fd)
{
    return mCtl.getUsedMemLock(type, fd);
}

int32_t SiriusCore::setMemStatus(RequestType type, int32_t fd, bool fresh)
{
    return mCtl.setMemStatus(type, fd, fresh);
}

int32_t SiriusCore::setMemSize(RequestType type, int32_t size)
{
    return mCtl.setMemSize(type, size);
}

int32_t SiriusCore::getMemSize(RequestType type, int32_t *size)
{
    return mCtl.getMemSize(type, size);
}

int32_t SiriusCore::lockMemory(RequestType type, int32_t fd)
{
    return mCtl.lockMemory(type, fd);
}

int32_t SiriusCore::addMemory(RequestType type, int32_t clientfd, bool fresh)
{
    return mCtl.addMemory(type, clientfd, fresh);
}

int32_t SiriusCore::unlockMemory(RequestType type, int32_t fd)
{
    return mCtl.unlockMemory(type, fd);
}

int32_t SiriusCore::setRequestedMark(RequestType type, bool enable)
{
    return mCtl.setRequest(type, enable);
}

int32_t SiriusCore::getHeader(Header &header)
{
    return mCtl.getHeader(header)
}

SiriusCore:: SiriusCore() :
    mConstructed(false),
    mModule(MODULE_SIRIUS_CORE),
    mExit(false),
    mCtlFd(-1),
    mCtlMem(NULL),
    mEvtSvr(NULL)
{
    for (int32_t i = 0; i < REQUEST_TYPE_MAX_INVALID; i++) {
        mRequests[i] = NULL;
        mCachedRequest[i] = false;
    }
}

SiriusCore::~SiriusCore()
{
    if (mConstructed) {
        destruct();
    }
}

int32_t SiriusCore::RunOnce::run(RunOnceFunc *func, void *in, void *out)
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

int32_t SiriusCore::RunOnce::exit()
{
    return RunOnceThread::exit();
}

bool SiriusCore::RunOnce::isRuning()
{
    return RunOnceThread::isRuning();
}

};

#include "PreviewRequestServer.h"
#include "YuvPictureRequestServer.h"

namespace sirius {

RequestHandler *SiriusCore::createRequestHandler(RequestType type)
{
    RequestHandler *request = NULL;

    switch (type) {
        case PREVIEW_NV21: {
            request = new PreviewRequestServer(this);
        } break;
        case PICTURE_NV21: {
            request = new YuvPictureRequestServer(this);
        } break;
        case PICTURE_BAYER: {
        } break;
        case EXTENDED_EVENT: {
            // EventRequestServer not inherited form RequestHandler.
        } break;
        default: {
            LOGE(mModule, "Invalid request type %d", type);
        } break;
    };

    return request;
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

