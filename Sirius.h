#ifndef _SIRIUS_H_
#define _SIRIUS_H_

#include "SiriusIntf.h"

namespace sirius {
    class SiriusImpl;
};

class Sirius :
    public SiriusIntf {
public:
    int32_t request(RequestType type);
    int32_t abort(RequestType type);
    int32_t enqueue(RequestType type, int32_t id);
    int32_t setCallback(RequestCbFunc requestCb, EventCbFunc eventCb);

public:
    Sirius();
    virtual ~Sirius();

private:
    Sirius(const Sirius &rhs);
    Sirius &operator=(const Sirius &rhs);

private:
    sirius::SiriusImpl *mImpl;
};

#endif
