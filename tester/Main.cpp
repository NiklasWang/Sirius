#include "common.h"
#include "ServerTester.h"

namespace sirius {

ServerTester gTester;

int32_t _test_main()
{
    int32_t rc = NO_ERROR;

    gTester.prepare();
    gTester.testUsecase(PREVIEW_NV21);
    gTester.testUsecase(PICTURE_NV21);
    gTester.testUsecase(EXTENDED_EVENT);

    return rc;
}

};

int32_t main()
{
    return sirius::_test_main();
}

