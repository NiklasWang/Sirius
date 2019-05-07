#include "BufferMgr.h"
#include "SiriusClientCore.h"

namespace sirius {

bool SiriusClientCore::kHeaderInited = false;
Header SiriusClientCore::kHeader;

SiriusClientCore::SiriusClientCore() :
    mConstructed(false),
    mModule(MODULE_SIRIUS_CLIENT_CORE),
    mConnected(false),
    mReady(false),
    mBufMgr(NULL)
{
    pthread_mutex_init(&mLocker, NULL);
}

SiriusClientCore::~SiriusClientCore()
{
    if (mConstructed) {
        destruct();
    }
    pthread_mutex_destroy(&mLocker);
}


int32_t SiriusClientCore::construct()
{
    int32_t rc = NO_ERROR;

    if (mConstructed) {
        LOGD(mModule, "Already constructed.");
        rc = ALREADY_INITED;
    }

    if (SUCCEED(rc)) {
        mBufMgr = new BufferMgr();
        if (ISNULL(mBufMgr)) {
            rc = NO_MEMORY;
            LOGE(mModule, "Failed to new buffer manager.");
        }
    }

    if (SUCCEED(rc)) {
        rc = mBufMgr->init();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to init buf manager, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = mSC.construct();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to construct ssm, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        mConstructed = true;
        LOGD(mModule, "Sirius client core constructed");
    }

    return rc;
 }

int32_t SiriusClientCore::prepare()
{
    int32_t rc = NO_ERROR;
    int32_t fd = -1;
    int32_t size = 0;
    bool locked  = false;
    char mSocketMsg[SOCKET_DATA_MAX_LEN];

    if (SUCCEED(rc)) {
        if (!kHeaderInited) {
            LOGE(mModule, "Please update sirius client core first.");
            rc = NOT_INITED;
        }
    }

    if (SUCCEED(rc)) {
        if (mReady) {
            LOGE(mModule, "Already prepared.");
            rc = ALREADY_INITED;
        }
    }

    if (SUCCEED(rc)) {
        pthread_mutex_lock(&mLocker);
        locked = true;
        if (mReady) {
            rc = ALREADY_INITED;
        }
    }

    if (SUCCEED(rc)) {
        if (!mConnected) {
            rc = mSC.connectServer();
            if (!SUCCEED(rc)) {
                LOGD(mModule, "Failed to connect server, "
                    "may not started, %s %d", strerror(errno), rc);
                rc = NOT_EXIST;
            } else {
                mConnected = true;
            }
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
            rc = BAD_PROTOCAL;
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
        mSocketMsg[0] = '\0';
        rc = mSC.receiveMsg(mSocketMsg, sizeof(mSocketMsg));
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to send msg \"%s\" to server, %d",
                SOCKET_CLIENT_REPLY_STR, rc);
        }
    }

    if (SUCCEED(rc)) {
        if (!COMPARE_SAME_STRING(mSocketMsg, SOCKET_CLIENT_REPLY_STR)) {
            LOGE(mModule, "Unknown msg received, \"%s\"", mSocketMsg);
            rc = NOT_READY;
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
        rc = mBufMgr->import(&mCtlBuf, fd, size);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to import %d bytes memory %d, %d",
                size, fd, rc);
        }
        if (ISNULL(mCtlBuf)) {
            LOGE(mModule, "Invalid import memory result.");
            rc = NO_MEMORY;
        }
    }

    if (SUCCEED(rc)) {
        rc = mCtl.init(mCtlBuf, size, false);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to set memory to controller, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = mCtl.setHeader(kHeader);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to set header to controller, %d", rc);
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

int32_t SiriusClientCore::destruct()
{
    int32_t rc = NO_ERROR;
    int32_t final = rc;

    if (!mConstructed) {
        rc = NOT_INITED;
    } else {
        mConstructed = false;
    }

    if (SUCCEED(rc)) {
        rc = mSC.destruct();
        if (!SUCCEED(rc)) {
            final |= rc;
            LOGE(mModule, "Failed to destruct socket state machine, %d", rc);
            RESETRESULT(rc);
        }
    }


    if (SUCCEED(rc)) {
        rc = releaseBuf(mCtlBuf);
        if (!SUCCEED(rc)) {
            final |= rc;
            LOGE(mModule, "Failed to release all mems, %d", rc);
            RESETRESULT(rc);
        }
    }

    if (SUCCEED(rc)) {
        mBufMgr->clear_all();
        rc = mBufMgr->deinit();
        if (!SUCCEED(rc)) {
            final |= rc;
            LOGE(mModule, "Failed to deinit ion buf mgr, %d", rc);
            RESETRESULT(rc);
        }
    }

    if (SUCCEED(rc)) {
        mReady = false;
        mConnected = false;
    }

    if (!SUCCEED(final)) {
        LOGE(mModule, "Sirius client core destructed with error %d", final);
    } else {
        LOGD(mModule, "Sirius client core destructed");
    }

    return rc;

}

int32_t SiriusClientCore::update(Header &header)
{
    int32_t rc = NO_ERROR;

    kHeader = header;
    kHeaderInited = true;

    if (ready()) {
        rc = mCtl.setHeader(kHeader);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to set header to controller, %d", rc);
        }
    }

    return rc;
}

bool SiriusClientCore::ready()
{
    return mReady;
}

bool SiriusClientCore::requested(RequestType type)
{
    return ready() && mCtl.requested(type);
}

int32_t SiriusClientCore::importBuf(void **buf, int32_t fd, int32_t len)
{
    return mBufMgr->import(buf, fd, len);
}

int32_t SiriusClientCore::flushBuf(void *buf)
{
    return mBufMgr->flush(buf);
}

int32_t SiriusClientCore::releaseBuf(void *buf)
{
    return mBufMgr->release_remove(buf);
}

int32_t SiriusClientCore::getUsedMem(RequestType type, int32_t *fd)
{
    return mCtl.getUsedMem(type, fd);
}

int32_t SiriusClientCore::setMemStatus(RequestType type, int32_t fd, bool fresh)
{
    return mCtl.setMemStatus(type, fd, fresh);
}

int32_t SiriusClientCore::getMemStatus(RequestType type, int32_t fd, bool *fresh)

{
    return mCtl.getMemStatus(type, fd, fresh);
}

int32_t SiriusClientCore::getMemSize(RequestType type, int32_t *size)
{
    return mCtl.getMemSize(type, size);
}

};

