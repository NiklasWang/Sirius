#include "common.h"
#include "ServerTester.h"

namespace sirius {

int32_t ServerTester::testUsecase(RequestType /*request*/)
{
    int32_t rc = NO_ERROR;

    return rc;
}

int32_t ServerTester::testPreview()
{
    int32_t rc = NO_ERROR;

    return rc;
}

int32_t ServerTester::testNV21Picture()
{
    int32_t rc = NO_ERROR;

    return rc;
}

int32_t ServerTester::testBayerPicture()
{
    int32_t rc = NO_ERROR;

    return rc;
}

int32_t ServerTester::testEvents()
{
    int32_t rc = NO_ERROR;

    return rc;
}

int32_t ServerTester::testCustomData()
{
    int32_t rc = NO_ERROR;

    return rc;
}

int32_t ServerTester::requestCbFunc(RequestType /*type*/, int32_t /*id*/, void * /*header*/, uint8_t * /*dat*/)
{
    int32_t rc = NO_ERROR;

    return rc;
}

int32_t ServerTester::eventCbFunc(int32_t /*event*/, int32_t /*arg1*/, int32_t /*arg2*/)
{
    int32_t rc = NO_ERROR;

    return rc;
}



static sirius::ServerTester gTester;

int32_t requestCbFunc(RequestType type, int32_t id, void *header, uint8_t *dat)
{
    return gTester.requestCbFunc(type, id, header, dat);
}

int32_t eventCbFunc(int32_t event, int32_t arg1, int32_t arg2)
{
    return gTester.eventCbFunc(event, arg1, arg2);
}

};
