#include "ClientTester.h"
#include "ClientTesterImpl.h"

#define CHECK_VALID_IMPL() \
    ({ \
        int32_t __rc = NO_ERROR; \
        if (ISNULL(mImpl)) { \
            LOGD(mModule, "Client tester impl not created"); \
            __rc = NOT_INITED; \
        } \
        __rc; \
    })

#define CONSTRUCT_IMPL() \
    ({ \
        int32_t __rc = NO_ERROR; \
        if (ISNULL(mImpl)) { \
            mImpl = new ClientTesterImpl(); \
            if (ISNULL(mImpl)) { \
                LOGE(mModule, "Failed to create client tester"); \
                __rc = NOT_INITED; \
            } else { \
                __rc = mImpl->construct(); \
                if (!SUCCEED(__rc)) { \
                    LOGE(mModule, "Failed to construct client tester"); \
                    delete mImpl; \
                    mImpl = NULL; \
                } else { \
                    LOGI(mModule, "Client tester constructed."); \
                } \
            } \
        } \
        __rc; \
    })


#define CONSTRUCT_IMPL_ONCE() \
    ({ \
        int32_t __rc = CHECK_VALID_IMPL(); \
        if (__rc == NOT_INITED) { \
            __rc = CONSTRUCT_IMPL(); \
        } \
        __rc; \
    })

namespace tester {

using namespace sirius;

int32_t ClientTester::prepare()
{
    int32_t rc = CONSTRUCT_IMPL_ONCE();
    return SUCCEED(rc) ? mImpl->prepare() : rc;
}

int32_t ClientTester::RunTest(RequestType type, int32_t round)
{
    int32_t rc = CONSTRUCT_IMPL_ONCE();
    return SUCCEED(rc) ? mImpl->RunTest(type, round) : rc;
}

int32_t ClientTester::getTestRounds(RequestType type)
{
    int32_t rc = CONSTRUCT_IMPL_ONCE();
    return SUCCEED(rc) ? mImpl->getTestRounds(type) : rc;
}

int32_t ClientTester::syncServerExit()
{
    int32_t rc = CONSTRUCT_IMPL_ONCE();
    return SUCCEED(rc) ? mImpl->syncServerExit() : rc;
}

ClientTester::ClientTester() :
    mImpl(NULL),
    mModule(MODULE_CLIENT_TESTER)
{
}

ClientTester::~ClientTester()
{
    if (NOTNULL(mImpl)) {
        SECURE_DELETE(mImpl);
    }
}

};

