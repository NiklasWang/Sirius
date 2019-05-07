#include "SiriusServer.h"
#include "SiriusServerImpl.h"

namespace sirius {

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
            mImpl = new SiriusServerImpl(); \
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

int32_t SiriusServer::request(RequestType type)
{
    int32_t rc = CONSTRUCT_IMPL_ONCE();
    return SUCCEED(rc) ? mImpl->request(type) : rc;
}

int32_t SiriusServer::abort(RequestType type)
{
    int32_t rc = CONSTRUCT_IMPL_ONCE();
    return SUCCEED(rc) ? mImpl->abort(type) : rc;
}

int32_t SiriusServer::enqueue(RequestType type, int32_t id)
{
    int32_t rc = CONSTRUCT_IMPL_ONCE();
    return SUCCEED(rc) ? mImpl->enqueue(type, id) : rc;
}

int32_t SiriusServer::setCallback(RequestCbFunc requestCb)
{
    int32_t rc = CONSTRUCT_IMPL_ONCE();
    return SUCCEED(rc) ? mImpl->setCallback(requestCb) : rc;
}

int32_t SiriusServer::setCallback(EventCbFunc eventCb)
{
    int32_t rc = CONSTRUCT_IMPL_ONCE();
    return SUCCEED(rc) ? mImpl->setCallback(eventCb) : rc;
}

int32_t SiriusServer::setCallback(DataCbFunc dataCb)
{
    int32_t rc = CONSTRUCT_IMPL_ONCE();
    return SUCCEED(rc) ? mImpl->setCallback(dataCb) : rc;
}

SiriusServer::SiriusServer() :
    mImpl(NULL)
{
}

SiriusServer::~SiriusServer()
{
    if (!ISNULL(mImpl)) {
        mImpl->destruct();
        delete mImpl;
        mImpl = NULL;
    }
}

};

