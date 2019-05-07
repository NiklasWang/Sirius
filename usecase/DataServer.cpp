#include "DataServer.h"
#include "DataRequestHeader.h"

namespace sirius {

int32_t DataServer::enqueue(int32_t /*id*/)
{
    // Not supported to enqueue from outside of Sirius
    return NOT_SUPPORTED;
}

int32_t DataServer::setSocketFd(int32_t fd)
{
    int32_t rc = NO_ERROR;

    if (SUCCEED(rc)) {
        if (mPrivateSocket) {
            rc = mPrivateSS.setClientFd(fd);
            if (!SUCCEED(rc)) {
                LOGE(mModule, "Failed to set fd to "
                    "private SS, %d", rc);
            } else {
                rc = JUMP_DONE;
            }
        }
    }

    if (SUCCEED(rc)) {
        if (!mPrivateSocket) {
            rc = RequestHandler::setSocketFd(fd);
            if (!SUCCEED(rc)) {
                LOGE(mModule, "Failed to set fd to "
                    "handler SS, %d", rc);
            } else {
                mPrivateSocket = true;
            }
        }
    }

    return rc;
};

int32_t DataServer::onClientReady()
{
    int32_t rc = NO_ERROR;

    if (mPrivateSocket) {
        rc = RequestHandler::onClientReady();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "On client ready failed, %d", rc);
        }
    }

    return rc;
}


int32_t DataServer::sendFreshData(
    RequestType type, int32_t id, void *head, void *dat)
{
    int32_t rc = NO_ERROR;
    DataRequestHeader *header = static_cast<DataRequestHeader *>(head);
    sp<MemoryHolderEx> holder = NULL;

    if (SUCCEED(rc)) {
        if (type != getType()) {
            LOGE(mModule, "Should NOT be here, %d/%d", type, getType());
            rc = BAD_PROTOCAL;
        }
    }

    if (SUCCEED(rc)) {
        rc = findMemoryHolder(header->type, header, &holder);
        if (!SUCCEED(rc) || ISNULL(holder)) {
            LOGE(mModule, "Failed to find memory holder, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = holder->add(header->block, dat, header->size, COPY_BUFFER);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to add to holder, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = RequestHandler::enqueue(id);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to enqueue buffer, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = mPrivateSS.sendMsg(
            SOCKET_DATA_NOTIFY_ENQUEUE,
            strlen(SOCKET_DATA_NOTIFY_ENQUEUE));
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to send msg %s, %d",
                SOCKET_DATA_NOTIFY_ENQUEUE, rc);
        }
    }

    return rc;
}

int32_t DataServer::findMemoryHolder(
    int32_t type, DataRequestHeader *header, sp<MemoryHolderEx> *holder)
{
    int32_t rc = NO_ERROR;
    sp<MemoryHolderEx> result = NULL;

    if (SUCCEED(rc)) {
        RWLock::AutoRLock l(mLock);
        for (auto &iter : mHolders) {
            if (iter->getType() == type) {
                result = iter;
                rc = JUMP_DONE;
                break;
            }
        }
    }

    if (SUCCEED(rc)) {
        if (ISNULL(result)) {
            result = new MemoryHolderEx(type, header->blocks, SAFE_HOLDER);
            if (ISNULL(result)) {
                LOGE(mModule, "Failed to new holder");
                rc = NO_MEMORY;
            }
        }
    }

    if (SUCCEED(rc)) {
        if (NOTNULL(result)) {
            rc = result->setCb(
                [this](int32_t type, void *data, int32_t size) -> int32_t {
                    return onDataCompleted(type, data, size);
                }
            );
            if (!SUCCEED(rc)) {
                LOGE(mModule, "Failed to set callback, %d", rc);
            }
        }
    }

    if (SUCCEED(rc)) {
        if (NOTNULL(result)) {
            RWLock::AutoWLock l(mLock);
            mHolders.push_back(result);
        }
    }

    if (rc == JUMP_DONE) {
        rc = RETURNIGNORE(rc, JUMP_DONE);
    }

    if (SUCCEED(rc)) {
        *holder = result;
    }

    return rc;
}

int32_t DataServer::onDataCompleted(
    int32_t type, void *data, int32_t size)
{
    int32_t rc = NO_ERROR;

    if (SUCCEED(rc)) {
        rc = mOps->send(type, data, size);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to send callback, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        RWLock::AutoWLock l(mLock);
        for (auto iter = mHolders.begin();
            iter != mHolders.end(); iter++) {
            if ((*iter)->getType() == type) {
                iter = mHolders.erase(iter);
                break;
            }
        }
    }

    return rc;
}

int32_t DataServer::getHeaderSize()
{
    return sizeof(DataRequestHeader);
}

int32_t DataServer::getDataSize()
{
    return DATA_TRANSMIT_MEMORY_SIZE;
}

int32_t DataServer::construct()
{
    int32_t rc = NO_ERROR;

    if (SUCCEED(rc)) {
       rc = mPrivateSS.construct();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to construct mPrivateSocket, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
      rc =  RequestHandler::construct();
      if (!SUCCEED(rc)) {
          LOGE(mModule, "Failed to construct RequestHandlerClient, %d", rc);
      }
    }

    return rc;
}

int32_t DataServer::destruct()
{
    int32_t rc = NO_ERROR;

    if (SUCCEED(rc)) {
       rc = mPrivateSS.destruct();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to destruct mPrivateSocket, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
      rc =  RequestHandler::destruct();
      if (!SUCCEED(rc)) {
          LOGE(mModule, "Failed to destruct RequestHandlerClient, %d", rc);
      }
    }

    return rc;
}

DataServer::DataServer(HandlerOpsIntf *ops) :
    RequestHandler(ops,
        CUSTOM_DATA,
        "Custom Data Request",
        CUSTOM_DATA_MEMORY_NUM),
    mModule(MODULE_DATA_REQUEST),
    mPrivateSocket(false)
{

}

DataServer::~DataServer()
{
    while(mHolders.begin() != mHolders.end()) {
        mHolders.erase(mHolders.begin());
    }
}

DataServer::MemoryHolderEx::MemoryHolderEx(
    int32_t type, int32_t blocks, bool safe) :
    MemoryHolder(type, blocks, safe)
{
}

};

