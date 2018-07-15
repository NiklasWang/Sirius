#ifndef _REQUEST_HANDLER_CLIENT_H_
#define _REQUEST_HANDLER_CLIENT_H_

#include "common.h"
#include "SiriusIntf.h"
#include "ThreadPoolEx.h"
#include "SiriusClientCore.h"
#include "SocketClientStateMachine.h"

namespace sirius {

class RequestHandlerClient
{
public:
    bool requested();
    int32_t onDataReady(void *header, uint8_t *dat);
    int32_t prepare();

protected:
    virtual int32_t sizeOfHeader() = 0;
    virtual int32_t sizeOfData(void *header) = 0;
    virtual int32_t copyDataToServer(uint8_t *dst, void *header, uint8_t *src) = 0;

protected:
    RequestHandlerClient(RequestType type, const char *name, uint32_t memNum);
    virtual ~RequestHandlerClient();
    int32_t construct();
    int32_t destruct();

private:
    const char *getName();
    RequestType getType();
    int32_t syncServerMemory();
    int32_t acceptSingleMemory();
    int32_t addMemoryMap(RequestType type,
        void *mem, int32_t fd, int32_t size);
    int32_t findMemoryMap(RequestType type,
        int32_t fd, void **mem, int32_t *size);
    int32_t releaseAllMems();
    int32_t notifyDataReady(int32_t fd);
    int32_t convertToRequestType(char *msg, const char *prefix, RequestType &result);

private:
    struct MemoryMap {
        int32_t fd;
        void   *mem;
        int32_t size;
    };

protected:
    bool        mConstructed;
    ModuleType  mModule;
    RequestType mType;
    const char *mName;
    bool        mConnected;
    bool        mReady;
    int32_t     mMemNum;
    int32_t     mMemMaxNum;
    MemoryMap  *mMemMap;
    pthread_mutex_t mLocker;
    SocketClientStateMachine mSC;

protected:
    static SiriusClientCore  kCore;
    static const RequestType kRequestTypeMap[REQUEST_TYPE_MAX_INVALID];
};

};

#endif
