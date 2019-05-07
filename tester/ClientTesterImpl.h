#ifndef _SIRIUS_CLIENT_TESTER_IMPL_H_
#define _SIRIUS_CLIENT_TESTER_IMPL_H_

#include "SiriusClient.h"
#include "ThreadPoolEx.h"
#include "SiriusClientCore.h"
#include "SocketClientStateMachine.h"
#include "RWLock.h"

namespace tester {

using namespace sirius;

class ClientTesterImpl :
    public noncopyable
{
public:
    int32_t prepare();
    int32_t RunTest(RequestType type, int32_t round);
    int32_t getTestRounds(RequestType type);
    int32_t syncServerExit();

public:
    ClientTesterImpl();
    virtual ~ClientTesterImpl();
    int32_t construct();
    int32_t destruct();

private:
    int32_t testMixture(int32_t round);
    int32_t testMixtureRound1(int32_t *count);
    int32_t testMixtureRound2(int32_t *count);
    int32_t testMixtureRound3(int32_t *count);

    int32_t testPreview(int32_t round);
    int32_t testPreviewRound(int32_t round,
        int32_t targetCount, int32_t msTime, int32_t *count);
    int32_t testPreviewMultiThread(int32_t threadNum,
        int32_t round, int32_t targetCount, int32_t msTime, int32_t *count);

    int32_t testPreviewRound1(int32_t *count);
    int32_t testPreviewRound2(int32_t *count);
    int32_t testPreviewRound3(int32_t *count);
    int32_t testPreviewRound4(int32_t *count);
    int32_t testPreviewRound5(int32_t *count);
    int32_t testPreviewRound6(int32_t *count);
    int32_t testPreviewRound7(int32_t *count);

    int32_t testNV21Picture(int32_t round);
    int32_t testNV21PictureRound(int32_t round,
        int32_t targetCount, int32_t msTime, int32_t *count);
    int32_t testNV21PictureMultiThread(int32_t threadNum,
        int32_t round, int32_t targetCount, int32_t msTime, int32_t *count);

    int32_t testNV21PictureRound1(int32_t *count);
    int32_t testNV21PictureRound2(int32_t *count);
    int32_t testNV21PictureRound3(int32_t *count);
    int32_t testNV21PictureRound4(int32_t *count);
    int32_t testNV21PictureRound5(int32_t *count);
    int32_t testNV21PictureRound6(int32_t *count);
    int32_t testNV21PictureRound7(int32_t *count);

    int32_t testBayerPicture(int32_t round);
    int32_t testBayerPictureRound(int32_t round,
        int32_t targetCount, int32_t msTime, int32_t *count);
    int32_t testBayerPictureMultiThread(int32_t threadNum,
        int32_t round, int32_t targetCount, int32_t msTime, int32_t *count);

    int32_t testBayerPictureRound1(int32_t *count);
    int32_t testBayerPictureRound2(int32_t *count);
    int32_t testBayerPictureRound3(int32_t *count);
    int32_t testBayerPictureRound4(int32_t *count);
    int32_t testBayerPictureRound5(int32_t *count);
    int32_t testBayerPictureRound6(int32_t *count);
    int32_t testBayerPictureRound7(int32_t *count);

    int32_t testEvents(int32_t round);
    int32_t testEventsRound(int32_t round,
        int32_t targetCount, int32_t msTime, int32_t *count);
    int32_t testEventsMultiThread(int32_t threadNum,
        int32_t round, int32_t targetCount, int32_t msTime, int32_t *count);

    int32_t testEventsRound1(int32_t *count);
    int32_t testEventsRound2(int32_t *count);
    int32_t testEventsRound3(int32_t *count);
    int32_t testEventsRound4(int32_t *count);
    int32_t testEventsRound5(int32_t *count);
    int32_t testEventsRound6(int32_t *count);
    int32_t testEventsRound7(int32_t *count);

    int32_t testCustomData(int32_t round);
    int32_t testCustomDataRound(int32_t round, uint8_t *data);
    int32_t testCustomDataSingleCases(int32_t *count, int32_t ms);
    int32_t testCustomDataSingleCasesReverse(int32_t *count, int32_t ms);
    int32_t testCustomDataRoundOneThread(int32_t *count, int32_t round, int32_t delayMs);
    int32_t testCustomDataRoundMultiThread(int32_t *count,
        int32_t round, int32_t delayRound, int32_t delayCase);

    int32_t testCustomDataRound1(int32_t *count);
    int32_t testCustomDataRound2(int32_t *count);
    int32_t testCustomDataRound3(int32_t *count);
    int32_t testCustomDataRound4(int32_t *count);
    int32_t testCustomDataRound5(int32_t *count);
    int32_t testCustomDataRound6(int32_t *count);
    int32_t testCustomDataRound7(int32_t *count);
    int32_t testCustomDataRound8(int32_t *count);
    int32_t testCustomDataRound9(int32_t *count);
    int32_t testCustomDataRound10(int32_t *count);
    int32_t testCustomDataRound11(int32_t *count);
    int32_t testCustomDataRound12(int32_t *count);
    int32_t testCustomDataRound13(int32_t *count);
    int32_t testCustomDataRound14(int32_t *count);
    int32_t testCustomDataRound15(int32_t *count);
    int32_t testCustomDataRound16(int32_t *count);

    int32_t SyncWithServerToStart(RequestType type, int32_t round);
    int32_t SyncWithServerToFinish(RequestType type, int32_t round, int32_t count);
    int32_t translateServerMsgToStatus(char *msg, const char *prefix, bool *succeed);

private:
    typedef int32_t (ClientTesterImpl::*TestFunction)(int32_t round);
    typedef int32_t (ClientTesterImpl::*TestCaseFunction)(int32_t *count);
    struct TestCases {
        RequestType  type;
        TestFunction func;
        int32_t      arg;
    };

private:
    ModuleType    mModule;
    bool          mConstructed;
    SiriusClient  mSirius;
    ThreadPoolEx *mThreads;
    RWLock        mSocketLocker;
    SocketClientStateMachine mSC;
    SocketClientStateMachine mBackupSC;

private:
    static const TestCases gTestCases[];
    static const TestCaseFunction gPreviewTestCases[];
    static const TestCaseFunction gPreviewSingleTestCases[];
    static const TestCaseFunction gNV21PictureTestCases[];
    static const TestCaseFunction gNV21PictureSingleTestCases[];
    static const TestCaseFunction gBayerPictureTestCases[];
    static const TestCaseFunction gBayerPictureSingleTestCases[];
    static const TestCaseFunction gEventTestCases[];
    static const TestCaseFunction gEventSingleTestCases[];
    static const TestCaseFunction gCustomDataTestCases[];
    static const TestCaseFunction gCustomDataSingleTestCases[];
    static const uint32_t         gCustomDataSingleTestCaseSize;
    static const TestCaseFunction gMixtureTestCases[];
};

};

#endif
