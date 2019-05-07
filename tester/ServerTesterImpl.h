#ifndef _SIRIUS_SERVER_TESTER_IMPL_H_
#define _SIRIUS_SERVER_TESTER_IMPL_H_

#include "common.h"
#include "SemaphoreTimeout.h"
#include "SiriusServer.h"
#include "ThreadPoolEx.h"
#include "SocketServerStateMachine.h"

namespace tester {

using namespace sirius;

class ServerTesterImpl :
    public noncopyable
{
public:
    int32_t start();

public:
    ServerTesterImpl();
    virtual ~ServerTesterImpl();
    int32_t construct();
    int32_t destruct();

private:
    int32_t cbRequest(RequestType type, int32_t id, void *header, void *dat);
    int32_t cbEvent(int32_t event, int32_t arg1, int32_t arg2);
    int32_t cbData(int32_t type, void *data, int32_t size);

    int32_t cbPreviewRound(int32_t round, int32_t id, PreviewNV21Header *header, uint8_t *dat);
    int32_t cbPreviewRound1(int32_t id, PreviewNV21Header *header, uint8_t *dat);
    int32_t cbPreviewRound2(int32_t id, PreviewNV21Header *header, uint8_t *dat);
    int32_t cbPreviewRound3(int32_t id, PreviewNV21Header *header, uint8_t *dat);
    int32_t cbPreviewRound4(int32_t id, PreviewNV21Header *header, uint8_t *dat);
    int32_t cbPreviewRound5(int32_t id, PreviewNV21Header *header, uint8_t *dat);
    int32_t cbPreviewRound6(int32_t id, PreviewNV21Header *header, uint8_t *dat);
    int32_t cbPreviewRound7(int32_t id, PreviewNV21Header *header, uint8_t *dat);

    int32_t cbNV21PictureRound(int32_t round, int32_t id, PictureNV21Header *header, uint8_t *dat);
    int32_t cbNV21PictureRound1(int32_t id, PictureNV21Header *header, uint8_t *dat);
    int32_t cbNV21PictureRound2(int32_t id, PictureNV21Header *header, uint8_t *dat);
    int32_t cbNV21PictureRound3(int32_t id, PictureNV21Header *header, uint8_t *dat);
    int32_t cbNV21PictureRound4(int32_t id, PictureNV21Header *header, uint8_t *dat);
    int32_t cbNV21PictureRound5(int32_t id, PictureNV21Header *header, uint8_t *dat);
    int32_t cbNV21PictureRound6(int32_t id, PictureNV21Header *header, uint8_t *dat);
    int32_t cbNV21PictureRound7(int32_t id, PictureNV21Header *header, uint8_t *dat);

    int32_t cbBayerPictureRound(int32_t round, int32_t id, PictureBayerHeader *header, uint8_t *dat);
    int32_t cbBayerPictureRound1(int32_t id, PictureBayerHeader *header, uint8_t *dat);
    int32_t cbBayerPictureRound2(int32_t id, PictureBayerHeader *header, uint8_t *dat);
    int32_t cbBayerPictureRound3(int32_t id, PictureBayerHeader *header, uint8_t *dat);
    int32_t cbBayerPictureRound4(int32_t id, PictureBayerHeader *header, uint8_t *dat);
    int32_t cbBayerPictureRound5(int32_t id, PictureBayerHeader *header, uint8_t *dat);
    int32_t cbBayerPictureRound6(int32_t id, PictureBayerHeader *header, uint8_t *dat);
    int32_t cbBayerPictureRound7(int32_t id, PictureBayerHeader *header, uint8_t *dat);

    int32_t cbEventRound(int32_t round, int32_t event, int32_t arg1, int32_t arg2);
    int32_t cbEventRound1(int32_t event, int32_t arg1, int32_t arg2);
    int32_t cbEventRound2(int32_t event, int32_t arg1, int32_t arg2);
    int32_t cbEventRound3(int32_t event, int32_t arg1, int32_t arg2);
    int32_t cbEventRound4(int32_t event, int32_t arg1, int32_t arg2);
    int32_t cbEventRound5(int32_t event, int32_t arg1, int32_t arg2);
    int32_t cbEventRound6(int32_t event, int32_t arg1, int32_t arg2);
    int32_t cbEventRound7(int32_t event, int32_t arg1, int32_t arg2);

    int32_t cbDataRound(int32_t round, int32_t type, void *data, int32_t size);
    int32_t cbDataRoundOneThread(int32_t type, void *data, int32_t size);
    int32_t cbDataRound1(int32_t type, void *data, int32_t size);
    int32_t cbDataRound2(int32_t type, void *data, int32_t size);
    int32_t cbDataRound3(int32_t type, void *data, int32_t size);
    int32_t cbDataRound4(int32_t type, void *data, int32_t size);
    int32_t cbDataRound5(int32_t type, void *data, int32_t size);
    int32_t cbDataRound6(int32_t type, void *data, int32_t size);
    int32_t cbDataRound7(int32_t type, void *data, int32_t size);
    int32_t cbDataRound8(int32_t type, void *data, int32_t size);
    int32_t cbDataRound9(int32_t type, void *data, int32_t size);
    int32_t cbDataRound10(int32_t type, void *data, int32_t size);
    int32_t cbDataRound11(int32_t type, void *data, int32_t size);
    int32_t cbDataRound12(int32_t type, void *data, int32_t size);
    int32_t cbDataRound13(int32_t type, void *data, int32_t size);
    int32_t cbDataRound14(int32_t type, void *data, int32_t size);
    int32_t cbDataRound15(int32_t type, void *data, int32_t size);
    int32_t cbDataRound16(int32_t type, void *data, int32_t size);

    int32_t cbMixtureRequest(RequestType type, int32_t id, void *header, void *dat);
    int32_t cbMixtureEvent(int32_t event, int32_t arg1, int32_t arg2);
    int32_t cbMixtureData(int32_t type, void *data, int32_t size);
    int32_t findMatchTypeSize(int32_t type, int32_t size);

private:
    int32_t resetSirius();
    int32_t runTest();
    int32_t runTests();
    int32_t startWaittingCb();
    int32_t convertMsgToRequest(char *msg,
        const char *prefix, RequestType *request, int32_t *round);
    int32_t convertMsgToStat(char *msg,
        const char *prefix, RequestType *request, int32_t *round, int32_t *count);

private:
   typedef int32_t (ServerTesterImpl::*cbPreviewFunc)(int32_t id, PreviewNV21Header *header, uint8_t *dat);
   typedef int32_t (ServerTesterImpl::*cbNV21PictureFunc)(int32_t id, PictureNV21Header *header, uint8_t *dat);
   typedef int32_t (ServerTesterImpl::*cbBayerPictureFunc)(int32_t id, PictureBayerHeader *header, uint8_t *dat);
   typedef int32_t (ServerTesterImpl::*cbEventFunc)(int32_t event, int32_t arg1, int32_t arg2);
   typedef int32_t (ServerTesterImpl::*cbDataFunc)(int32_t type, void *data, int32_t size);

private:
    friend int32_t cbRequestCFunc(RequestType type, int32_t id, void *header, void *dat);
    friend int32_t cbEventCFunc(int32_t event, int32_t arg1, int32_t arg2);
    friend int32_t cbDataCFunc(int32_t type, void *data, int32_t size);

private:
    ModuleType    mModule;
    bool          mConstructed;
    RequestType   mTestingRequest;
    int32_t       mTestingRound;
    SiriusServer  mSirius;
    ThreadPoolEx *mThreads;
    SocketServerStateMachine mSS;
    SocketServerStateMachine mBackupSS;

private:
    int32_t mPreviewCnt;
    bool    mPreviewResult;
    uint8_t mPreviewIdentity;
    SemaphoreTimeout mPreviewSem;
    int32_t mNV21PictureCnt;
    bool    mNV21PictureResult;
    uint8_t mNV21PictureIdentity;
    SemaphoreTimeout mNV21PictureSem;
    int32_t mBayerPictureCnt;
    bool    mBayerPictureResult;
    uint8_t mBayerPictureIdentity;
    SemaphoreTimeout mBayerPictureSem;
    int32_t mEventCnt;
    bool    mEventResult;
    int32_t mLastEvent;
    SemaphoreTimeout mEventSem;
    int32_t mDataCnt;
    bool    mDataResult;
    SemaphoreTimeout mDataSem;
    int32_t mMixtureCnt;
    bool    mMixtureResult;
    SemaphoreTimeout mMixtureSem;

private:
    static const cbPreviewFunc      gPreviewCbs[];
    static const cbNV21PictureFunc  gNV21PictureCbs[];
    static const cbBayerPictureFunc gBayerPictureCbs[];
    static const cbEventFunc gEventCbs[];
    static const cbDataFunc  gDataCbs[];
};

};

#endif
