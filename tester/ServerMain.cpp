#include "common.h"
#include "ServerTester.h"

namespace tester {

ServerTester gTester;

using namespace sirius;

int32_t _test_main()
{
    int32_t rc = NO_ERROR;

    rc = gTester.start();
    if (!SUCCEED(rc)) {
        LOGE(MODULE_TESTER, "Some error occurs, "
            "check previous log, %d", rc);
    }

    return rc;
}

};

int32_t main()
{
    return tester::_test_main();
}

