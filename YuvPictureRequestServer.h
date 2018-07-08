#ifndef _YUV_PICTURE_REQUEST_SERVER_H_
#define _YUV_PICTURE_REQUEST_SERVER_H_

#include "common.h"
#include "RequestHandler.h"

namespace sirius {

class YuvPictureRequestServer :
    public RequestHandler,
    public noncopyable {
public:
    int32_t getHeaderSize() override;
    int32_t getDataSize() override;

public:
    YuvPictureRequestServer(HandlerOpsIntf *ops);
    virtual ~YuvPictureRequestServer();
};

};

#endif
