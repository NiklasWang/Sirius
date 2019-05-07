#include <unistd.h>

#include "common.h"
#include "PandoraServerTester.h"

namespace sirius {

extern ServerTester gTester;

int32_t _test_main()
{
    int32_t rc = NO_ERROR;

    gTester.prepare();
    gTester.testUsecase(PREVIEW_NV21);
    gTester.testUsecase(PICTURE_NV21);
    gTester.testUsecase(PICTURE_BAYER);
    gTester.testUsecase(EXTENDED_EVENT);
    gTester.testUsecase(CUSTOM_DATA);

    while(1){
        sleep(1);
    }

    return rc;
}

};

int32_t main()
{
    return sirius::_test_main();
}

