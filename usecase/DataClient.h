#ifndef _DATA_REQUEST_CLIENT_H_
#define _DATA_REQUEST_CLIENT_H_

#include "modules.h"
#include "SiriusServerIntf.h"
#include "common.h"
#include "RefBase.h"
#include "RWLock.h"
#include "RequestHandlerClient.h"
#include "SemaphoreTimeout.h"

namespace sirius {

class DataClient :
    public RequestHandlerClient,
    public noncopyable {
public:
    int32_t onDataReady(int32_t type, void *dat, int32_t size);

public:
    int32_t sizeOfHeader() override;
    int32_t sizeOfData(void *header) override;
    int32_t copyDataToServer(uint8_t *dst, void *header, uint8_t *src) override;

public:
    int32_t construct();
    int32_t destruct();
    DataClient();
    virtual ~DataClient();

private:
    int32_t onBlockReady(int32_t type,
        int32_t block, int32_t blocks, void *data, int32_t size);
    int32_t waitUntilReady();
    int32_t waitTypeToFinish(int32_t type);
    int32_t notifyTypeToFinish(int32_t type);
    int32_t startAvailabilityChecker();
    int32_t checkAvailabilityLoop();
    int32_t checkAvailability(char *msg);
    int32_t maxDataSize();

private:
    struct TypeSem :
        public RefBase {
        int32_t type;
        Semaphore *sem;

    public:
        TypeSem();
        ~TypeSem();
    };

private:
    ModuleType mModule;
    bool       mConstructed;
    bool       mAvailability;
    SemaphoreTimeout mSem;
    SocketClientStateMachine mPrivateSM;
    ThreadPoolEx    *mThreads;
    RWLock           mListLock;
    std::list<sp<TypeSem> > mTypeSems;
};

};

#endif

