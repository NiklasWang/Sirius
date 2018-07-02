#include <sys/socket.h>

#include "common.h"
#include "IonBufferMgr.h"
#include "socket_server.h"

using namespace sirius;

#define SHARE_MEMORY_SIZE 1024*1024
#define TEST_MSG_STR      "Hello, Niklas."

static IonBufferMgr gIonMgr;
static void   *gBuf = NULL;
static int32_t gIonFd = -1;
static int32_t gSockFd = -1;
static int32_t gClientFd = -1;

struct share_data {
    int32_t cnt;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    char msg[32];
};

static share_data *gData = NULL;

int32_t main()
{
    int32_t rc = NO_ERROR;

    if (SUCCEED(rc)) {
        rc = gIonMgr.init();
        if (!SUCCEED(rc)) {
            LOGE(MODULE_TESTER, "Failed to init ion buf mgr, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = gIonMgr.allocate(&gBuf, SHARE_MEMORY_SIZE, &gIonFd);
        if (!SUCCEED(rc)) {
            LOGE(MODULE_TESTER, "Failed to alloc %d bytes ion buf", SHARE_MEMORY_SIZE);
        }
        if (ISNULL(gBuf)) {
            LOGD(MODULE_TESTER, "Buf is NULL");
        } else {
            gData = static_cast<share_data *>(gBuf);
            gData->cnt = 0xdeadbeef;
            strcpy(gData->msg, TEST_MSG_STR);
            pthread_mutex_init(&gData->mutex, NULL);
            pthread_cond_init(&gData->cond, NULL);
        }
    }

    if (SUCCEED(rc)) {
        rc = start_server(&gSockFd);
        if (!SUCCEED(rc)) {
            LOGE(MODULE_TESTER, "Failed to start server");
        }
    }

    if (SUCCEED(rc)) {
        int32_t abort = false;
        LOGD(MODULE_TESTER, "Wait for client connection...");
        rc = poll_accept_wait(gSockFd, &gClientFd, &abort);
        if (!SUCCEED(rc)) {
            LOGE(MODULE_TESTER, "Failed to poll data while sleeping, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = sc_send_fd(gClientFd, gIonFd);
        if (!SUCCEED(rc)) {
            LOGE(MODULE_TESTER, "Failed to send fd %d to client", gIonFd);
        }
    }

    if (SUCCEED(rc)) {
        pthread_mutex_lock(&gData->mutex);
        LOGI(MODULE_TESTER, "Will lock and sleep for 5s...");
        sleep(5);
        LOGI(MODULE_TESTER, "Wake up and unlock...");
        pthread_mutex_unlock(&gData->mutex);
    }

    if (SUCCEED(rc)) {
        pthread_mutex_lock(&gData->mutex);
        LOGI(MODULE_TESTER, "Will wait for signal here...");
        pthread_cond_wait(&gData->cond, &gData->mutex);
        LOGI(MODULE_TESTER, "Signal received and continue.");
        pthread_mutex_unlock(&gData->mutex);
    }

    if (SUCCEED(rc)) {
        gIonMgr.release(gBuf);
        shutdown(gClientFd, SHUT_RDWR);
        close(gClientFd);
        shutdown(gSockFd, SHUT_RDWR);
        close(gSockFd);
        close(gIonFd);
        gBuf = NULL;
    }

    return rc;
}
