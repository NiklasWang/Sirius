#include <sys/socket.h>

#include "common.h"
#include "IonBufferMgr.h"
#include "socket_client.h"

using namespace sirius;

#define SHARE_MEMORY_SIZE 1024*1024

static IonBufferMgr gIonMgr;
static void   *gBuf = NULL;
static int32_t gIonFd = -1;
static int32_t gSockFd = -1;

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
            LOGD(MODULE_TESTER, "Failed to init ion buf mgr, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = connect_to_server(&gSockFd);
        if (!SUCCEED(rc)) {
            LOGE(MODULE_TESTER, "Failed to connect server, may not started, %d", rc);
        } else {
            LOGD(MODULE_TESTER, "Server connected.");
        }
    }

    if (SUCCEED(rc)) {
        int32_t abort = false;
        LOGD(MODULE_TESTER, "Wait for server to send ion fd");
        rc = poll_server_fd_wait(gSockFd, &gIonFd, &abort);
        if (!SUCCEED(rc)) {
            LOGE(MODULE_TESTER, "Failed to poll fd while sleeping, %d", rc);
        }
    }

    if (SUCCEED(rc)) {
        rc = gIonMgr.import(&gBuf, gIonFd, SHARE_MEMORY_SIZE);
        if (!SUCCEED(rc)) {
            LOGE(MODULE_TESTER, "Failed to import memory for fd %d, %d", gIonFd, rc);
        } else {
            LOGD(MODULE_TESTER, "Memory fd %d mapped %p", gIonFd, gBuf);
        }
        if (ISNULL(gBuf)) {
            LOGD(MODULE_TESTER, "Buf is NULL and that's invalid");
            rc = UNKNOWN_ERROR;
        } else {
            gData = static_cast<share_data *>(gBuf);
        }
    }

    if (SUCCEED(rc)) {
        sleep(1);
        LOGI(MODULE_TESTER, "Test data is 0x%x", gData->cnt);
        LOGI(MODULE_TESTER, "Shared message is \"%s\"", gData->msg);
        LOGI(MODULE_TESTER, "Will lock after 4s...");
        pthread_mutex_lock(&gData->mutex);
        LOGI(MODULE_TESTER, "locked.");
        pthread_mutex_unlock(&gData->mutex);
    }

    if (SUCCEED(rc)) {
        pthread_mutex_lock(&gData->mutex);
        LOGI(MODULE_TESTER, "Will send signal here...");
        pthread_cond_signal(&gData->cond);
        LOGI(MODULE_TESTER, "Signal sent.");
        pthread_mutex_unlock(&gData->mutex);
    }

    if (SUCCEED(rc)) {
        gIonMgr.release(gBuf);
        shutdown(gSockFd, SHUT_RDWR);
        close(gSockFd);
        close(gIonFd);
        gBuf = NULL;
    }

    return rc;
}

