#ifndef _BAYER_PICTURE_REQUEST_SERVER_H_
#define _BAYER_PICTURE_REQUEST_SERVER_H_

#include "common.h"
#include "RequestHandler.h"

namespace sirius {

class BayerPictureServer :
    public RequestHandler,
    public noncopyable {
public:
    int32_t getHeaderSize() override;
    int32_t getDataSize() override;

public:
    BayerPictureServer(HandlerOpsIntf *ops);
    virtual ~BayerPictureServer();
};

};

#endif
