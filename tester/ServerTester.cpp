#include "ServerTester.h"
#include "ServerTesterImpl.h"

namespace tester {

ServerTesterImpl gTester;

int32_t ServerTester::start()
{
    int32_t rc = NO_ERROR;
    if (SUCCEED(rc)) {
        rc = gTester.start();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to start impl, %d", rc);
        }
    }

    return rc;
}

ServerTester::ServerTester() :
    mModule(MODULE_SERVER_TESTER)
{
}

ServerTester::~ServerTester()
{
}

};

