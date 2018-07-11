#ifndef _SIRIUS_SOCKET_CLIENT_STATE_MACHINE__
#define _SIRIUS_SOCKET_CLIENT_STATE_MACHINE__

#include "common.h"
#include "SyncType.h"
#include "QueuedThread.h"

namespace sirius {

class SocketClientStateMachine :
    public QueuedThread,
    public noncopyable {
public:
    int32_t connectServer();
    int32_t receiveMsg(char *data, int32_t maxlen);
    int32_t sendMsg(const char *data, int32_t len);
    int32_t receiveFd(int32_t *fd);
    int32_t sendFd(int32_t fd);
    int32_t cancelWaitMsg();
    bool connected();

private:
    enum cmd_type {
        CMD_CONNECT_SERVER,
        CMD_RECEIVE_MSG,
        CMD_SEND_MSG,
        CMD_RECEIVE_FD,
        CMD_SEND_FD,
    };

    enum status {
        STATUS_UNINITED,
        STATUS_INITED,
        STATUS_SERVER_CONNECTED,
        STATUS_RECEIVING_MSG,
        STATUS_SENDING_MSG,
        STATUS_RECEIVING_FD,
        STATUS_SENDING_FD,
    };

    struct msg_info {
        char *msg;
        int32_t len;
        int32_t max_len;
    };

    struct cmd_info {
        cmd_type cmd;
        union {
            msg_info *msg;
            int32_t *fd;
        } u;
        int32_t  rc;
        SyncType sync;
    };

    virtual int32_t processTask(void *dat) override;
    virtual int32_t taskDone(void *dat, int32_t rc) override;

    int32_t stateMachine(cmd_type cmd, void *arg);
    int32_t procCmdUninitedState(cmd_type cmd, void *arg);
    int32_t procCmdInitedState(cmd_type cmd, void *arg);
    int32_t procCmdServerConnectedState(cmd_type cmd, void *arg);
    int32_t procCmdReceivingMsgState(cmd_type cmd, void *arg);
    int32_t procCmdSendingMsgState(cmd_type cmd, void *arg);
    int32_t procCmdReceivingFdState(cmd_type cmd, void *arg);
    int32_t procCmdSendingFdState(cmd_type cmd, void *arg);
    int32_t executeOnThread(cmd_info *task);

    const char *stateName(status stat);
    const char *cmdName(cmd_type type);
    void logInvalidCmd(cmd_type cmd);
    void updateToNewStatus(status state);

public:
    SocketClientStateMachine();
    virtual ~SocketClientStateMachine();
    int32_t construct();
    int32_t destruct();

private:
    typedef int32_t (SocketClientStateMachine::*SMFunc)(cmd_type cmd, void *arg);

private:
    bool       mConstructed;
    int32_t    mServerFd;
    status     mStatus;
    ModuleType mModule;
    int32_t    mCancelWait;
    static const SMFunc mFunc[];
    static const char * const kStateStr[];
    static const char * const kCmdStr[];
};

};

#endif

