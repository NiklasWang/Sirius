#include "SiriusIntfData.h"

namespace sirius {

static const RequestType gRequestTypeMap[] = {
    [PREVIEW_NV21]   = PREVIEW_NV21,
    [PICTURE_NV21]   = PICTURE_NV21,
    [PICTURE_BAYER]  = PICTURE_BAYER,
    [EXTENDED_EVENT] = EXTENDED_EVENT,
    [CUSTOM_DATA]    = CUSTOM_DATA,
    [REQUEST_TYPE_MAX_INVALID] = REQUEST_TYPE_MAX_INVALID,
};

static const char * const gRequestNameMap[] = {
    [PREVIEW_NV21]   = "preview(nv21) request",
    [PICTURE_NV21]   = "picture(nv21) request",
    [PICTURE_BAYER]  = "picture(bayer) request",
    [EXTENDED_EVENT] = "extended event request",
    [CUSTOM_DATA]    = "custom data request",
    [REQUEST_TYPE_MAX_INVALID] = "max invalid request",
};


RequestType getRequestType(RequestType type)
{
    return (type < 0 ||
        type > REQUEST_TYPE_MAX_INVALID) ?
        REQUEST_TYPE_MAX_INVALID : type;
}

RequestType convertToRequestType(int32_t index)
{
    return (index < 0 ||
        index > REQUEST_TYPE_MAX_INVALID) ?
        REQUEST_TYPE_MAX_INVALID : gRequestTypeMap[index];
}

const char *getRequestName(RequestType type)
{
    return gRequestNameMap[getRequestType(type)];
}

};
