#ifndef _PREVIEW_REQUEST_SERVER_H_
#define _PREVIEW_REQUEST_SERVER_H_

#include "common.h"
#include "RequestHandler.h"

namespace sirius {

class PreviewRequestServer :
    public RequestHandler,
    public noncopyable {
public:
    int32_t getHeaderSize() override;
    int32_t getDataSize() override;

public:
    PreviewRequestServer(HandlerOpsIntf *ops);
    virtual ~PreviewRequestServer();
};

};

#endif
