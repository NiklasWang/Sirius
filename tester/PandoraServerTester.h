#ifndef _SIRIUS_PANDORA_SERVER_TESTER_H_
#define _SIRIUS_PANDORA_SERVER_TESTER_H_

#include "SiriusServer.h"

namespace sirius {

class ServerTester
{
public:
    int32_t prepare();
    int32_t testUsecase(RequestType request);
    int32_t abortUsecase(RequestType request);

public:
    int32_t requestCb(RequestType type, int32_t id, void *header, void *dat);
    int32_t eventCb(int32_t event, int32_t arg1, int32_t arg2);
    int32_t dataCb(int32_t type, void *data, int32_t size);

public:
    ServerTester();
    virtual ~ServerTester();

private:
    int32_t testPreview();
    int32_t testNV21Picture();
    int32_t testBayerPicture();
    int32_t testEvents();
    int32_t testCustomData();
    int32_t abortPreview();
    int32_t abortNV21Picture();
    int32_t abortBayerPicture();
    int32_t abortEvents();
    int32_t abortCustomData();

private:
    ModuleType   mModule;
    SiriusServer mSirius;
};

};

#endif
