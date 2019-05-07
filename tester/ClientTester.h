#ifndef _SIRIUS_CLIENT_TESTER_H_
#define _SIRIUS_CLIENT_TESTER_H_

#include "common.h"
#include "SiriusClientIntf.h"

namespace tester {

using namespace sirius;

class ClientTesterImpl;

class ClientTester :
    public noncopyable
{
public:
    int32_t prepare();
    int32_t RunTest(RequestType type, int32_t round);
    int32_t getTestRounds(RequestType type);
    int32_t syncServerExit();

public:
    ClientTester();
    virtual ~ClientTester();

private:
    ClientTesterImpl *mImpl;
    ModuleType        mModule;
};

};

#endif
