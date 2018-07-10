#include "Sirius.h"
#include "SiriusImpl.h"

using namespace sirius;

#define CHECK_VALID_IMPL() \
    ({ \
        int32_t __rc = NO_ERROR; \
        if (ISNULL(mImpl)) { \
            LOGD(MODULE_SIRIUS, "Sirius impl not created"); \
            __rc = NOT_INITED; \
        } \
        __rc; \
    })

#define CONSTRUCT_IMPL() \
    ({ \
        int32_t __rc = NO_ERROR; \
        if (ISNULL(mImpl)) { \
            mImpl = new SiriusImpl(); \
            if (ISNULL(mImpl)) { \
                LOGE(MODULE_SIRIUS, "Failed to create Sirius impl"); \
                __rc = NOT_INITED; \
            } else { \
                __rc = mImpl->construct(); \
                if (!SUCCEED(__rc)) { \
                    LOGE(MODULE_SIRIUS, "Failed to construct Sirius impl"); \
                    delete mImpl; \
                    mImpl = NULL; \
                } else { \
                    LOGI(MODULE_SIRIUS, "Sirius impl constructed."); \
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

int32_t Sirius::request(RequestType type)
{
    int32_t rc = CONSTRUCT_IMPL_ONCE();
    return SUCCEED(rc) ? mImpl->request(type) : rc;
}

int32_t Sirius::abort(RequestType type)
{
    int32_t rc = CONSTRUCT_IMPL_ONCE();
    return SUCCEED(rc) ? mImpl->abort(type) : rc;
}

int32_t Sirius::enqueueBuf(RequestType type, int32_t id)
{
    int32_t rc = CONSTRUCT_IMPL_ONCE();
    return SUCCEED(rc) ? mImpl->enqueueBuf(type, id) : rc;
}

int32_t Sirius::setCallback(RequestCbFunc requestCb, EventCbFunc eventCb)
{
    int32_t rc = CONSTRUCT_IMPL_ONCE();
    return SUCCEED(rc) ? mImpl->setCallback(requestCb, eventCb) : rc;
}

Sirius::Sirius() :
    mImpl(NULL)
{
}

Sirius::~Sirius()
{
    if (!ISNULL(mImpl)) {
        mImpl->destruct();
        delete mImpl;
        mImpl = NULL;
    }
}

