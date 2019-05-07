#include "common.h"
#include "SiriusClient.h"
#include "SiriusClientImpl.h"
#include "RWLock.h"

namespace sirius {

#define CHECK_VALID_IMPL() \
    ({ \
        int32_t __rc = NO_ERROR; \
        if (ISNULL(mImpl)) { \
            LOGD(MODULE_SIRIUS, "Sirius client impl not created"); \
            __rc = NOT_INITED; \
        } \
        __rc; \
    })

#define CONSTRUCT_IMPL() \
    ({ \
        int32_t __rc = NO_ERROR; \
        if (ISNULL(mImpl)) { \
            mImpl = new SiriusClientImpl(); \
            if (ISNULL(mImpl)) { \
                LOGE(MODULE_SIRIUS, "Failed to create Sirius client impl"); \
                __rc = NOT_INITED; \
            } \
        } \
        __rc; \
    })

static RWLock gLock;

#define CONSTRUCT_IMPL_ONCE() \
    ({ \
        RWLock::AutoWLock l(gLock); \
        int32_t __rc = CHECK_VALID_IMPL(); \
        if (__rc == NOT_INITED) { \
            __rc = CONSTRUCT_IMPL(); \
        } \
        __rc; \
    })

int32_t  override;


int32_t SiriusClient::init(Header &header)
{
    int32_t rc = CONSTRUCT_IMPL_ONCE();
    return SUCCEED(rc) ? mImpl->init(header) : rc;
}

int32_t SiriusClient::onPreviewReady(int32_t w, int32_t h,
    int32_t stride, int32_t scanline, void *data, int64_t ts)
{
    int32_t rc = CONSTRUCT_IMPL_ONCE();
    return SUCCEED(rc) ? mImpl->onPreviewReady(
        w, h, stride, scanline, data, ts) : rc;
}

int32_t SiriusClient::onYuvPictureReady(int32_t w, int32_t h,
    int32_t stride, int32_t scanline, void *data, int64_t ts)
{
    int32_t rc = CONSTRUCT_IMPL_ONCE();
    return SUCCEED(rc) ? mImpl->onYuvPictureReady(
        w, h, stride, scanline, data, ts) : rc;
}

int32_t SiriusClient::onBayerPictureReady(int32_t w, int32_t h,
    void *data, int64_t ts, Pattern pattern)
{
    int32_t rc = CONSTRUCT_IMPL_ONCE();
    return SUCCEED(rc) ? mImpl->onBayerPictureReady(
        w, h, data, ts, pattern) : rc;
}

int32_t SiriusClient::sendEvent(int32_t evt, int32_t arg1, int32_t arg2)
{
    int32_t rc = CONSTRUCT_IMPL_ONCE();
    return SUCCEED(rc) ? mImpl->sendEvent(evt, arg1, arg2) : rc;
}

int32_t SiriusClient::sendData(int32_t type, void *data, int32_t size)
{
    int32_t rc = CONSTRUCT_IMPL_ONCE();
    return SUCCEED(rc) ? mImpl->sendData(type, data, size) : rc;
}

int32_t SiriusClient::abort(int32_t type)
{
    int32_t rc = CONSTRUCT_IMPL_ONCE();
    return SUCCEED(rc) ? mImpl->abort(type) : rc;
}


SiriusClient::SiriusClient() :
    mImpl(NULL)
{
}

SiriusClient::~SiriusClient()
{
    if (NOTNULL(mImpl)) {
        SECURE_DELETE(mImpl);
    }
}

};

