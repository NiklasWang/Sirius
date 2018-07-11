#ifndef _EVENT_REQUEST_SERVER_H_
#define _EVENT_REQUEST_SERVER_H_

#include "common.h"
#include "RequestHandler.h"

namespace sirius {

class EventServer :
    public RequestHandler,
    public noncopyable {
public:
    int32_t getHeaderSize() override;
    int32_t getDataSize() override;
    int32_t runOnceFunc(void *in, void *out) override;

public:
    EventServer(HandlerOpsIntf *ops);
    virtual ~EventServer();

private:
    struct EvtInfo {
        EvtInfo();
        int32_t evt;
        int32_t arg1;
        int32_t arg2;
    };

private:
    int32_t convertMsgToEvt(char *msg, EvtInfo &evt);
    int32_t onNewEvent(EvtInfo &info);
};

};

#endif
