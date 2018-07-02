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
    int32_t enqueueBuf(RequestType type, void *buf, int32_t size);
    int32_t dequeueBuf(RequestType type, void *buf = ALL_BUFFERS);
    int32_t setCallback(callback_func func);

public:
    Sirius();
    virtual ~Sirius();

private:
    Sirius(const Sirius &rhs);
    Sirius &operator =(const Sirius &rhs);

private:
    sirius::SiriusImpl *mImpl;
};

#endif
