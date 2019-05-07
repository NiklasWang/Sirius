#include "TimeEx.h"
#include "ClientTester.h"

#define MODULE MODULE_CLIENT_TESTER

namespace tester {

using namespace sirius;

ClientTester gTester;

int32_t ClientStressMain()
{
    int32_t rc = NO_ERROR;

    if (SUCCEED(rc)) {
        do {
            rc = gTester.prepare();
            if (rc == NOT_READY) {
                LOGI(MODULE, "Server may not started, retry in 1 seconds...");
                delaySec(1);
            } else if (!SUCCEED(rc)) {
                LOGE(MODULE, "Failed to connect to server, %d", rc);
                break;
            }
        } while (rc == NOT_READY);
    }

    if (SUCCEED(rc)) {
        for (int32_t i = 0; i <= REQUEST_TYPE_MAX_INVALID; i++) {
            RequestType request = convertToRequestType(i);
            int32_t rounds = gTester.getTestRounds(request);
            for (int32_t j = 0; j < rounds; j++) {
                rc = gTester.RunTest(request, j);
                if (!SUCCEED(rc)) {
                    LOGE(MODULE, "Run client test failure, request %s round %d, %d",
                        getRequestName(request), j, rc);
                    break;
                } else {
                    LOGI(MODULE, "Run client test request %s round %d succeed.",
                        getRequestName(request), j);
                }
            }
            if (!SUCCEED(rc)) {
                break;
            }
        }
    }

    if (SUCCEED(rc) || !SUCCEED(rc)) {
        rc = gTester.syncServerExit();
        if (!SUCCEED(rc)) {
            LOGE(MODULE, "Failed to sync server exit, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        LOGI(MODULE, "Succeed to run all client tests! Congratulations!");
    } else {
        LOGE(MODULE, "Failed to run client tests, check previous log.");
    }

    return rc;
}

};

int32_t main()
{
    return tester::ClientStressMain();
}

