#ifndef _DATA_REQUEST_SERVER_H_
#define _DATA_REQUEST_SERVER_H_

#include "common.h"
#include "RequestHandler.h"
#include "RefBase.h"
#include "MemoryHolder.h"
#include "DataRequestHeader.h"

namespace sirius {

class DataServer :
    public RequestHandler,
    public noncopyable {
public:
    int32_t getHeaderSize() override;
    int32_t getDataSize() override;
    int32_t setSocketFd(int32_t fd) override;
    int32_t onClientReady() override;
    int32_t enqueue(int32_t id) override;
    int32_t sendFreshData(RequestType type,
        int32_t id, void *head, void *dat) override;

public:
    DataServer(HandlerOpsIntf *ops);
    virtual ~DataServer();
    int32_t construct() override;
    int32_t destruct() override;

private:
    class MemoryHolderEx;
    int32_t findMemoryHolder(int32_t type,
        DataRequestHeader *header, sp<MemoryHolderEx> *holder);
    int32_t onDataCompleted(
        int32_t type, void *data, int32_t size);

private:
    class MemoryHolderEx :
        public MemoryHolder,
        public RefBase {
    public:
        MemoryHolderEx(int32_t type,
        int32_t blocks = NO_BLOCK_LIMIT, bool safe = NON_SAFE_HOLDER);
    };

private:
    ModuleType mModule;
    bool       mPrivateSocket;
    SocketServerStateMachine mPrivateSS;
    RWLock     mLock;
    std::list<sp<MemoryHolderEx> > mHolders;
};

};

#endif
