#ifndef _SIRIUS_SERVER_H_
#define _SIRIUS_SERVER_H_

#include "SiriusServerIntf.h"

namespace sirius {

class SiriusServerImpl;

class SiriusServer :
    public SiriusServerIntf {
public:
    int32_t request(RequestType type);
    int32_t abort(RequestType type);
    int32_t enqueue(RequestType type, int32_t id);

    int32_t setCallback(RequestCbFunc requestCb);
    int32_t setCallback(EventCbFunc eventCb);
    int32_t setCallback(DataCbFunc dataCb);

public:
    SiriusServer();
    virtual ~SiriusServer();

private:
    SiriusServer(const SiriusServer &rhs);
    SiriusServer &operator=(const SiriusServer &rhs);

private:
    SiriusServerImpl *mImpl;
};

};

#endif
