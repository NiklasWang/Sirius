#ifndef _EVENT_REQUEST_CLIENT_H_
#define _EVENT_REQUEST_CLIENT_H_

#include "common.h"
#include "SiriusIntf.h"
#include "configuration.h"
#include "SocketClientStateMachine.h"

namespace sirius {

class SiriusClientCore;

class EventClient :
    public noncopyable {
public:
    int32_t sendEvent(int32_t evt, int32_t arg1, int32_t arg2);

public:
    EventClient();
    ~EventClient();
    int32_t construct();
    int32_t destruct();

public:
    RequestType type();
    const char *name();

private:
    struct EvtInfo;
    int32_t convertEvtToMsg(char *msg, EvtInfo *evt);

private:
    struct EvtInfo {
        EvtInfo(int32_t evt, int32_t arg1, int32_t arg2);
        int32_t evt;
        int32_t arg1;
        int32_t arg2;
    };


private:
    bool        mConstructed;
    ModuleType  mModule;
    RequestType mType;
    char        mMsg[SOCKET_DATA_MAX_LEN];
    const char * const mName;
    SiriusClientCore  *mCore;
    SocketClientStateMachine mSCSM;
};

};

#endif

