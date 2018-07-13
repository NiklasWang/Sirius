#ifndef _SIRIUS_SERVER_TESTER_H_
#define _SIRIUS_SERVER_TESTER_H_

#include "Sirius.h"

namespace sirius {

class ServerTester
{
public:
    int32_t testUsecase(RequestType request);

public:
    int32_t requestCbFunc(RequestType type, int32_t id, void *header, uint8_t *dat);
    int32_t eventCbFunc(int32_t event, int32_t arg1, int32_t arg2);

private:
    int32_t testPreview();
    int32_t testNV21Picture();
    int32_t testBayerPicture();
    int32_t testEvents();
    int32_t testCustomData();


private:
    Sirius mSirius;
};

};

#endif
