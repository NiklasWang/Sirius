#ifndef _PREVIEW_REQUEST_SERVER_H_
#define _PREVIEW_REQUEST_SERVER_H_

#include "common.h"
#include "RequestHandler.h"

namespace sirius {

class PreviewServer :
    public RequestHandler,
    public noncopyable {
public:
    int32_t getHeaderSize() override;
    int32_t getDataSize() override;

public:
    PreviewServer(HandlerOpsIntf *ops);
    virtual ~PreviewServer();
};

};

#endif
