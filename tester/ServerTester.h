#ifndef _SIRIUS_SERVER_TESTER_H_
#define _SIRIUS_SERVER_TESTER_H_

#include "Sirius.h"

namespace sirius {

class ServerTester
{
public:
    int32_t prepare();
    int32_t testUsecase(RequestType request);

public:
    int32_t requestCb(RequestType type, int32_t id, void *header, void *dat);
    int32_t eventCb(int32_t event, int32_t arg1, int32_t arg2);

public:
    ServerTester();
    virtual ~ServerTester();

private:
    int32_t testPreview();
    int32_t testNV21Picture();
    int32_t testBayerPicture();
    int32_t testEvents();
    int32_t testCustomData();

private:
    ModuleType mModule;
    Sirius     mSirius;
};

};

#endif
