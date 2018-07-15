#ifndef _EVENT_REQUEST_CLIENT_H_
#define _EVENT_REQUEST_CLIENT_H_

#include "modules.h"
#include "SiriusIntf.h"
#include "common.h"
#include "RequestHandlerClient.h"

namespace sirius {

class EventClient :
    public RequestHandlerClient,
    public noncopyable {
public:
    int32_t onEventReady(int32_t event, int32_t arg1, int32_t arg2);

public:
    int32_t sizeOfHeader() override;
    int32_t sizeOfData(void *header) override;
    int32_t copyDataToServer(uint8_t *dst, void *header, uint8_t *src) override;

public:
    int32_t construct();
    int32_t destruct();
    EventClient();
    virtual ~EventClient();

private:
    ModuleType  mModule;
};

};

#endif

