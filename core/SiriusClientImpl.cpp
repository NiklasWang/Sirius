#include "SiriusServerIntf.h"
#include "SiriusClientCore.h"
#include "PreviewClient.h"
#include "YuvPictureClient.h"
#include "BayerPictureClient.h"
#include "EventClient.h"
#include "DataClient.h"
#include "SiriusClientImpl.h"

namespace sirius {

int32_t SiriusClientImpl::init(Header &header)
{
    int32_t rc = NO_ERROR;

    if (SUCCEED(rc)) {
        if (ISNULL(mCore)) {
            mCore = new SiriusClientCore();
            if (ISNULL(mCore)) {
                LOGE(mModule, "Failed to create client core");
                rc = NO_MEMORY;
            }
        }
    }

    if (SUCCEED(rc)) {
        rc = mCore->update(header);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to update header, %d", rc);
        }
    }

    return rc;
}

int32_t SiriusClientImpl::onPreviewReady(int32_t w, int32_t h,
    int32_t stride, int32_t scanline, void *data, int64_t ts)
{
    int32_t rc = NO_ERROR;

    if (ISNULL(mPreview)) {
        RWLock::AutoWLock l(mLock);
        rc = enablePreview();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to enable preview, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = mPreview->onPreviewReady(w, h,
            stride, scanline, data, ts);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to send to preview client, %d", rc);
        }
    }

    return rc;
}

int32_t SiriusClientImpl::onYuvPictureReady(
    int32_t w, int32_t h, int32_t stride,
    int32_t scanline, void *data, int64_t ts)
{
    int32_t rc = NO_ERROR;

    if (ISNULL(mSnapshot)) {
        RWLock::AutoWLock l(mLock);
        rc = enableSnapshot();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to enable snapshot, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = mSnapshot->onYuvPictureReady(w, h,
            stride, scanline, data, ts);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to send to snapshot client, %d", rc);
        }
    }

    return rc;
}

int32_t SiriusClientImpl::onBayerPictureReady(
    int32_t w, int32_t h, void *data, int64_t ts, Pattern pattern)
{
    int32_t rc = NO_ERROR;

    if (ISNULL(mBayer)) {
        RWLock::AutoWLock l(mLock);
        rc = enableBayer();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to enable bayer picture, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = mBayer->onBayerPictureReady(w, h, data, ts, pattern);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to send to bayer client, %d", rc);
        }
    }

    return rc;
}

int32_t SiriusClientImpl::sendEvent(int32_t evt, int32_t arg1, int32_t arg2)
{
    int32_t rc = NO_ERROR;

    if (ISNULL(mEvent)) {
        RWLock::AutoWLock l(mLock);
        rc = enableEvent();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to enable event, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = mEvent->onEventReady(evt, arg1, arg2);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to send to event client, %d", rc);
        }
    }

    return rc;
}

int32_t SiriusClientImpl::sendData(int32_t type, void *data, int32_t size)
{
    int32_t rc = NO_ERROR;

    if (ISNULL(mData)) {
        RWLock::AutoWLock l(mLock);
        rc = enableData();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to enable data, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = mData->onDataReady(type, data, size);
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to send to data client, %d", rc);
        }
    }

    return rc;
}

int32_t SiriusClientImpl::abort(int32_t type) {
    int32_t rc = NO_ERROR;
    RWLock::AutoWLock l(mLock);

    switch (type) {
        case PREVIEW_NV21:
            if (NOTNULL(mPreview)) {
                rc = mPreview->destruct();
                SECURE_DELETE(mPreview);
            }
            break;
        case PICTURE_NV21:
            if (NOTNULL(mSnapshot)) {
                rc = mSnapshot->destruct();
                SECURE_DELETE(mSnapshot);
            }
            break;
        case PICTURE_BAYER:
            if (NOTNULL(mBayer)) {
                rc = mBayer->destruct();
                SECURE_DELETE(mBayer);
            }
            break;
        case EXTENDED_EVENT:
            if (NOTNULL(mEvent)) {
                rc = mEvent->destruct();
                SECURE_DELETE(mEvent);
            }
            break;
        case CUSTOM_DATA:
            if (NOTNULL(mData)) {
                rc = mData->destruct();
                SECURE_DELETE(mData);
            }
            break;
        case REQUEST_TYPE_MAX_INVALID:
            if (NOTNULL(mPreview)) {
                rc = mPreview->destruct();
                SECURE_DELETE(mPreview);
            }
            if (NOTNULL(mSnapshot)) {
                rc = mSnapshot->destruct();
                SECURE_DELETE(mSnapshot);
            }
            if (NOTNULL(mBayer)) {
                rc = mBayer->destruct();
                SECURE_DELETE(mBayer);
            }
            if (NOTNULL(mEvent)) {
                rc = mEvent->destruct();
                SECURE_DELETE(mEvent);
            }
            if (NOTNULL(mData)) {
                rc = mData->destruct();
                SECURE_DELETE(mData);
            }
            break;
        default :
            LOGE(mModule, "Invalid type: %d", type);
            break;
    }

    return rc;
}

int32_t SiriusClientImpl::enablePreview()
{
    int32_t rc = NO_ERROR;

    if (NOTNULL(mPreview)) {
        LOGE(mModule, "Already created.");
        rc = ALREADY_EXISTS;
    }

    if (SUCCEED(rc)) {
        mPreview = new PreviewClient();
        if (ISNULL(mPreview)) {
            LOGE(mModule, "Failed to create preview client");
            rc = NO_MEMORY;
        }
    }

    if (SUCCEED(rc)) {
        rc = mPreview->construct();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to construct preview client, %d", rc);
        }
    }

    if (!SUCCEED(rc)) {
        if (NOTNULL(mPreview)) {
            SECURE_DELETE(mPreview);
        }
    }

    return rc;
}

int32_t SiriusClientImpl::enableSnapshot()
{
    int32_t rc = NO_ERROR;

    if (NOTNULL(mSnapshot)) {
        LOGE(mModule, "Already created.");
        rc = ALREADY_EXISTS;
    }

    if (SUCCEED(rc)) {
        mSnapshot = new YuvPictureClient();
        if (ISNULL(mSnapshot)) {
            LOGE(mModule, "Failed to create snapshot client");
            rc = NO_MEMORY;
        }
    }

    if (SUCCEED(rc)) {
        rc = mSnapshot->construct();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to construct snapshot client, %d", rc);
        }
    }

    if (!SUCCEED(rc)) {
        if (NOTNULL(mSnapshot)) {
            SECURE_DELETE(mSnapshot);
        }
    }

    return rc;
}

int32_t SiriusClientImpl::enableBayer()
{
    int32_t rc = NO_ERROR;

    if (NOTNULL(mBayer)) {
        LOGE(mModule, "Already created.");
        rc = ALREADY_EXISTS;
    }

    if (SUCCEED(rc)) {
        mBayer = new BayerPictureClient();
        if (ISNULL(mBayer)) {
            LOGE(mModule, "Failed to create bayer picture client");
            rc = NO_MEMORY;
        }
    }

    if (SUCCEED(rc)) {
        rc = mBayer->construct();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to construct bayer picture client, %d", rc);
        }
    }

    if (!SUCCEED(rc)) {
        if (NOTNULL(mBayer)) {
            SECURE_DELETE(mBayer);
        }
    }

    return rc;
}

int32_t SiriusClientImpl::enableEvent()
{
    int32_t rc = NO_ERROR;

    if (NOTNULL(mEvent)) {
        LOGE(mModule, "Already created.");
        rc = ALREADY_EXISTS;
    }

    if (SUCCEED(rc)) {
        mEvent = new EventClient();
        if (ISNULL(mEvent)) {
            LOGE(mModule, "Failed to create event client");
            rc = NO_MEMORY;
        }
    }

    if (SUCCEED(rc)) {
        rc = mEvent->construct();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to construct event client, %d", rc);
        }
    }

    if (!SUCCEED(rc)) {
        if (NOTNULL(mEvent)) {
            SECURE_DELETE(mEvent);
        }
    }

    return rc;
}

int32_t SiriusClientImpl::enableData()
{
    int32_t rc = NO_ERROR;

    if (NOTNULL(mData)) {
        LOGE(mModule, "Already created.");
        rc = ALREADY_EXISTS;
    }

    if (SUCCEED(rc)) {
        mData = new DataClient();
        if (ISNULL(mData)) {
            LOGE(mModule, "Failed to create data client");
            rc = NO_MEMORY;
        }
    }

    if (SUCCEED(rc)) {
        rc = mData->construct();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to construct data client, %d", rc);
        }
    }

    if (!SUCCEED(rc)) {
        if (NOTNULL(mData)) {
            SECURE_DELETE(mData);
        }
    }

    return rc;
}

SiriusClientImpl::SiriusClientImpl() :
    mModule(MODULE_SIRIUS),
    mPreview(NULL),
    mSnapshot(NULL),
    mBayer(NULL),
    mEvent(NULL),
    mData(NULL),
    mCore(NULL)
{
}

SiriusClientImpl::~SiriusClientImpl()
{
    if (NOTNULL(mCore)) {
        SECURE_DELETE(mCore);
    }
    if (NOTNULL(mPreview)) {
        mPreview->destruct();
        SECURE_DELETE(mPreview);
    }
    if (NOTNULL(mSnapshot)) {
        mSnapshot->destruct();
        SECURE_DELETE(mSnapshot);
    }
    if (NOTNULL(mBayer)) {
        mBayer->destruct();
        SECURE_DELETE(mBayer);
    }
    if (NOTNULL(mEvent)) {
        mEvent->destruct();
        SECURE_DELETE(mEvent);
    }
    if (NOTNULL(mData)) {
        mData->destruct();
        SECURE_DELETE(mData);
    }
}

};

