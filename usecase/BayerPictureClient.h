#ifndef _BAYER_PICTURE_REQUEST_CLIENT_H_
#define _BAYER_PICTURE_REQUEST_CLIENT_H_

#include "modules.h"
#include "SiriusServerIntf.h"
#include "common.h"
#include "RequestHandlerClient.h"

namespace sirius {

class BayerPictureClient :
    public RequestHandlerClient,
    public noncopyable {
public:
    int32_t onBayerPictureReady(int32_t w, int32_t h,
        void *data, int64_t ts = 0LL, Pattern pattern = BAYER_PATTERN_RGGB);

public:
    int32_t sizeOfHeader() override;
    int32_t sizeOfData(void *header) override;
    int32_t copyDataToServer(uint8_t *dst, void *header, uint8_t *src) override;

public:
    int32_t construct();
    int32_t destruct();
    BayerPictureClient();
    virtual ~BayerPictureClient();

private:
    ModuleType  mModule;
};

};

#endif
