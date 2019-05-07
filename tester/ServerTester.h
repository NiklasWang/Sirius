#ifndef _SIRIUS_SERVER_TESTER_H_
#define _SIRIUS_SERVER_TESTER_H_

#include "common.h"

namespace tester {

class ServerTesterImpl;

class ServerTester :
    public sirius::noncopyable
{
public:
    int32_t start();

public:
    ServerTester();
    virtual ~ServerTester();

private:
    sirius::ModuleType mModule;
};

};

#endif
