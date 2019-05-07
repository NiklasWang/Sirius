#include "common.h"
#include "DataClient.h"
#include "DataRequestHeader.h"

#define CLIENT_MAX_WAIT_TIME 5000 // ms

namespace sirius {

int32_t DataClient::onDataReady(int32_t type, void *data, int32_t size)
{
    int32_t rc = NO_ERROR;

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
        static RWLock gSmLock;
        RWLock::AutoWLock l(gSmLock);
        if (!mAvailability) {
            rc = startAvailabilityChecker();
            if (!SUCCEED(rc)) {
                LOGE(mModule, "Failed to start availability checker.");
            }else{
                mAvailability = true;
            }
        }
    }

    if (SUCCEED(rc)) {
        rc = waitTypeToFinish(type);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to wait same type to finish, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        int32_t sizeLeft  = size;
        int32_t blockSize = maxDataSize();
        int32_t blocks    = size / blockSize + (size % blockSize != 0);
        for (int32_t i = 0; i < blocks; i++) {
            int32_t curSize = sizeLeft > blockSize ? blockSize : sizeLeft;
            void   *curData = (void *)((int8_t *)data + i * blockSize);
            rc = onBlockReady(type, i, blocks, curData, curSize);
            if (SUCCEED(rc)) {
                sizeLeft -= blockSize;
            } else {
                LOGE(mModule, "Failed to send block data, %d", rc);
            }
        }
    }

    if (SUCCEED(rc)) {
        rc = notifyTypeToFinish(type);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to notify same type finished, %d", rc);
        }
    }

    return rc;
}

int32_t DataClient::onBlockReady(int32_t type,
    int32_t block, int32_t blocks, void *data, int32_t size)
{
    int32_t rc = NO_ERROR;

    if (SUCCEED(rc)) {
        rc = waitUntilReady();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to wait until ready, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        DataRequestHeader header = {
            .type   = type,
            .block  = block,
            .blocks = blocks,
            .size   = size,
        };
        rc = RequestHandlerClient::onDataReady(&header, (uint8_t *)data);
        if (!SUCCEED(rc)) {
            LOGD(mModule, "Failed to copy frame data, %d", rc);
        }
    }

    return rc;
}

int32_t DataClient::waitTypeToFinish(int32_t type)
{
    int32_t rc = NO_ERROR;
    sp<TypeSem> sync = NULL;

    if (SUCCEED(rc)) {
        RWLock::AutoRLock l(mListLock);
        for (auto &iter : mTypeSems) {
            if (iter->type == type) {
                sync = iter;
                break;
            }
        }
    }

    if (SUCCEED(rc)) {
        if (ISNULL(sync)) {
            sync = new TypeSem();
            sync->type = type;
            if (ISNULL(sync)) {
                LOGE(mModule, "Failed to new type sem.");
                rc = NO_MEMORY;
            } else {
                RWLock::AutoWLock l(mListLock);
                mTypeSems.push_back(sync);
            }
        }
    }

    if (SUCCEED(rc)) {
        rc = sync->sem->wait();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to wait same type to finish, %d", rc);
        }
    }

    return rc;
}

int32_t DataClient::notifyTypeToFinish(int32_t type)
{
    int32_t rc = NO_ERROR;
    sp<TypeSem> sync = NULL;

    if (SUCCEED(rc)) {
        rc = NOT_FOUND;
        RWLock::AutoRLock l(mListLock);
        for (auto &iter : mTypeSems) {
            if (iter->type == type) {
                sync = iter;
                rc = NO_ERROR;
                break;
            }
        }
        ASSERT_LOG(mModule, NOTNULL(sync),
            "Sem not found, suicide...");
    }

    if (SUCCEED(rc)) {
        if (NOTNULL(sync)) {
            sync->sem->signal();
        }
    }

    return rc;
}

int32_t DataClient::waitUntilReady()
{
    int32_t rc = mSem.wait();
    if (rc == ETIMEDOUT) {
        LOGE(mModule, "Wait timedout in %s ms.", CLIENT_MAX_WAIT_TIME);
    } else if (!SUCCEED(rc)) {
        LOGE(mModule, "Failed to wait, %d", rc);
    }

    return rc;
}

int32_t DataClient::startAvailabilityChecker()
{
    int32_t rc = NO_ERROR;

    rc = mThreads->run(
        [this]() -> int32_t {
            return checkAvailabilityLoop();
        }
    );
    if (!SUCCEED(rc)) {
        LOGE(mModule, "Failed to run in thread, %d", rc);
    }

    return rc;
}

int32_t DataClient::checkAvailabilityLoop()
{
    int32_t rc = NO_ERROR;
    char msg[SOCKET_DATA_MAX_LEN];

    if (SUCCEED(rc)) {
        rc = mPrivateSM.connectServer();
        if (!SUCCEED(rc)) {
            LOGD(mModule, "Failed to connect server, should "
                "NOT be here since server already started, %s %d",
                strerror(errno), rc);
            rc = NOT_EXIST;
        }
    }

    if (SUCCEED(rc)) {
        char msg[SOCKET_DATA_MAX_LEN];
        sprintf(msg, SOCKET_CLIENT_CONNECT_TYPE " %d", getType());
        rc = mPrivateSM.sendMsg(msg, strlen(msg));
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to send msg \"%s\" to server, %d",
                msg, rc);
        }
    }

    if (SUCCEED(rc)) {
        do {
            rc = checkAvailability(msg);
            if (rc == USER_ABORTED) {
                LOGI(mModule, "User aborted to check availability");
                break;
            } else if (!SUCCEED(rc)) {
                LOGE(mModule, "Failed to check availability, %d", rc);
                break;
            }
        } while (rc != USER_ABORTED);
    }

    return rc;
}


int32_t DataClient::checkAvailability(char *msg)
{
    int32_t rc = NO_ERROR;

    if (SUCCEED(rc)) {
        msg[0] = '\0';
        rc = mPrivateSM.receiveMsg(msg, SOCKET_DATA_MAX_LEN);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to receive msg, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        int32_t len = strlen(SOCKET_DATA_NOTIFY_ENQUEUE);
        for (int32_t i = 0; i < SOCKET_DATA_MAX_LEN; i++) {
            if (msg[i * len] == '\0') {
                break;
            }
            if (COMPARE_SAME_LEN_STRING(msg + i * len,
                SOCKET_DATA_NOTIFY_ENQUEUE, len)) {
                mSem.signal();
            }
        }
    }

    return rc;
}

DataClient::DataClient() :
    RequestHandlerClient(
        CUSTOM_DATA,
        "Extended Data Request Client",
        REQUEST_HANDLER_MAX_MEMORY_NUM),
    mModule(MODULE_DATA_REQUEST),
    mConstructed(false),
    mAvailability(false),
    mSem(CLIENT_MAX_WAIT_TIME, CUSTOM_DATA_MEMORY_NUM),
    mThreads(NULL)
{
}

DataClient::~DataClient()
{
    destruct();
}

int32_t DataClient::construct()
{
    int32_t rc = NO_ERROR;

    if (mConstructed) {
        rc = ALREADY_INITED;
    }

    if (SUCCEED(rc)) {
        mThreads = ThreadPoolEx::getInstance();
        if (ISNULL(mThreads)) {
            LOGE(mModule, "Failed to get thread pool");
            rc = NOT_READY;
        }
    }

    if (SUCCEED(rc)) {
        rc = mPrivateSM.construct();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to construct private sm, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = RequestHandlerClient::construct();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to construct data client, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        mConstructed = true;
        LOGD(mModule, "Data client constructed");
    }

    return rc;
}

int32_t DataClient::destruct()
{
    int32_t rc = NO_ERROR;
    int32_t final = rc;

    if (!mConstructed) {
        rc = NOT_INITED;
    } else {
        mConstructed = false;
    }

    if(mAvailability){
        mAvailability = false;
        mPrivateSM.cancelWaitMsg();
    }

    if (SUCCEED(rc)) {
        if (NOTNULL(mThreads)) {
            mThreads->removeInstance();
            mThreads = NULL;
        }
    }

    if (SUCCEED(rc)) {
        rc = mPrivateSM.destruct();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to destruct private sm, %d", rc);
            final |= rc;
        }
    }

    if (SUCCEED(rc)) {
        rc = RequestHandlerClient::destruct();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to destruct handler client, %d", rc);
            final |= rc;
        }
    }

    if (!SUCCEED(final)) {
        LOGE(mModule, "Data client handler %s destructed "
            "with error %d", getName(), final);
    } else {
        LOGD(mModule, "Data client %s destructed", getName());
    }

    return rc;
}

int32_t DataClient::sizeOfHeader()
{
    return sizeof(DataRequestHeader);
}

int32_t DataClient::sizeOfData(void *header)
{
    DataRequestHeader *_header =
        static_cast<DataRequestHeader *>(header);
    int32_t size = _header->size;

    if (size > maxDataSize()) {
        LOGE(mModule, "Should NOT be here, memory overflow, %d/%d",
            _header->size, maxDataSize());
        size = maxDataSize();
    }

    return size;
}

int32_t DataClient::copyDataToServer(
    uint8_t * dst, void *header, uint8_t *src)
{
    DataRequestHeader *_header =
        static_cast<DataRequestHeader *>(header);

    memcpy(dst, src, _header->size);

    return NO_ERROR;
}

int32_t DataClient::maxDataSize()
{
    return DATA_TRANSMIT_MEMORY_SIZE;
}

DataClient::TypeSem::TypeSem()
{
    sem = new Semaphore(1);
    ASSERT_LOG(MODULE_DATA_REQUEST, sem != NULL,
        "No memory to new semaphore");
}

DataClient::TypeSem::~TypeSem()
{
    if (NOTNULL(sem)) {
        SECURE_DELETE(sem);
    }
}


};

