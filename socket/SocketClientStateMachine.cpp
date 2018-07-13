#include "SocketClientStateMachine.h"
#include "socket_client.h"
#include "socket_server.h"

namespace sirius {

int32_t SocketClientStateMachine::connectServer()
{
    return stateMachine(CMD_CONNECT_SERVER, NULL);
}

int32_t SocketClientStateMachine::receiveMsg(char *data, int32_t maxlen)
{
    msg_info msg = {
        .msg = data,
        .len = 0,
        .max_len = maxlen,
    };
    return stateMachine(CMD_RECEIVE_MSG, &msg);
}

int32_t SocketClientStateMachine::sendMsg(const char *data, int32_t msglen)
{
    msg_info msg = {
        .msg = const_cast<char *>(data),
        .len = msglen,
        .max_len = msglen,
    };
    return stateMachine(CMD_SEND_MSG, &msg);
}

int32_t SocketClientStateMachine::receiveFd(int32_t *fd)
{
    return stateMachine(CMD_RECEIVE_FD, fd);
}

int32_t SocketClientStateMachine::sendFd(int32_t fd)
{
    int32_t copied_fd = fd;
    return stateMachine(CMD_SEND_FD, &copied_fd);
}

int32_t SocketClientStateMachine::cancelWaitMsg()
{
    // Don't post to state machine
    mCancelWait = true;
    return NO_ERROR;
}

bool SocketClientStateMachine::connected()
{
    return mServerFd != -1;
}

SocketClientStateMachine::SocketClientStateMachine() :
    mConstructed(false),
    mServerFd(-1),
    mStatus(STATUS_UNINITED),
    mModule(MODULE_SOCKET_CLIENT_SM),
    mCancelWait(0)
{
}

SocketClientStateMachine::~SocketClientStateMachine()
{
    if (mConstructed) {
        destruct();
    }
}

int32_t SocketClientStateMachine::construct()
{
    int32_t rc = NO_ERROR;

    if (mConstructed) {
        rc = ALREADY_INITED;
    }

    if (SUCCEED(rc)) {
        rc = constructThread();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to construct thread");
        }
    }

    if (SUCCEED(rc)) {
        mStatus = STATUS_INITED;
        mConstructed = true;
    }

    return RETURNIGNORE(rc, ALREADY_INITED);
}

int32_t SocketClientStateMachine::destruct()
{
    int32_t rc = NO_ERROR;

    if (!mConstructed) {
        rc = NOT_INITED;
    } else {
        mConstructed = false;
    }

    if (SUCCEED(rc)) {
        rc = destructThread();
        if (!SUCCEED(rc)) {
            LOGE(mModule, "Failed to destruct thread");
        }
    }

    if (SUCCEED(rc)) {
        if (mServerFd > 0) {
            disconnect_server(mServerFd);
            mServerFd = -1;
        }
    }

    return RETURNIGNORE(rc, NOT_INITED);
}

int32_t SocketClientStateMachine::processTask(void *dat)
{
    int32_t rc = NO_ERROR;
    cmd_info *info = static_cast<cmd_info *>(dat);

    switch (info->cmd) {
        case CMD_CONNECT_SERVER: {
            rc = connect_to_server(&mServerFd);
            if (!SUCCEED(rc)) {
                LOGD(mModule, "Failed to connect server, "
                    "may not started, %d", rc);
            }
        } break;
        case CMD_RECEIVE_MSG: {
            char *data = info->u.msg->msg;
            int32_t *len = &info->u.msg->len;
            int32_t max_len = info->u.msg->max_len;
            data[0] = '\0';
            rc = poll_read_wait(mServerFd, data, max_len, len, &mCancelWait);
            if (!SUCCEED(rc)) {
                LOGE(mModule, "Failed to poll data while sleeping, %d", rc);
            }
        } break;
        case CMD_SEND_MSG: {
            char *data = info->u.msg->msg;
            int32_t len = info->u.msg->len;
            rc = sc_send_data(mServerFd, data, len, CLIENT);
            if (!SUCCEED(rc)) {
                LOGE(mModule, "Failed to send data, %d", rc);
            }
        } break;
        case CMD_RECEIVE_FD: {
            int32_t *fd = info->u.fd;
            rc = poll_server_fd_wait(mServerFd, fd, &mCancelWait);
            if (!SUCCEED(rc)) {
                LOGE(mModule, "Failed to poll fd while sleeping, %d", rc);
            }
        } break;
        case CMD_SEND_FD: {
            int32_t fd = *info->u.fd;
            rc = sc_send_fd(mServerFd, fd);
            if (!SUCCEED(rc)) {
                LOGE(mModule, "Failed to send fd %d to server", fd);
            }
        } break;
        default: {
            LOGE(mModule, "Invalid cmd %d", info->cmd);
        } break;
    }

    return rc;
}

int32_t SocketClientStateMachine::taskDone(void *dat, int32_t result)
{
    int32_t rc = NO_ERROR;
    cmd_info *info = static_cast<cmd_info *>(dat);

    rc = info->rc = result;

    if (info->sync == SYNC_TYPE) {
        info->sync.signal();
    }

    return rc;
}

const SocketClientStateMachine::SMFunc
    SocketClientStateMachine::mFunc[] = {
    [STATUS_UNINITED]         = &SocketClientStateMachine::procCmdUninitedState,
    [STATUS_INITED]           = &SocketClientStateMachine::procCmdInitedState,
    [STATUS_SERVER_CONNECTED] = &SocketClientStateMachine::procCmdServerConnectedState,
    [STATUS_RECEIVING_MSG]    = &SocketClientStateMachine::procCmdReceivingMsgState,
    [STATUS_SENDING_MSG]      = &SocketClientStateMachine::procCmdSendingMsgState,
    [STATUS_RECEIVING_FD]     = &SocketClientStateMachine::procCmdReceivingFdState,
    [STATUS_SENDING_FD]       = &SocketClientStateMachine::procCmdSendingFdState,
};

int32_t SocketClientStateMachine::stateMachine(
    SocketClientStateMachine::cmd_type cmd, void *arg)
{
    return (this->*(mFunc[mStatus]))(cmd, arg);
}

int32_t SocketClientStateMachine::executeOnThread(
    SocketClientStateMachine::cmd_info *task)
{
    return newTask((void *)task);
}

const char * const SocketClientStateMachine::kStateStr[] = {
    [STATUS_UNINITED]         = "uninited",
    [STATUS_INITED]           = "inited",
    [STATUS_SERVER_CONNECTED] = "server connected",
    [STATUS_RECEIVING_MSG]    = "receiving message",
    [STATUS_SENDING_MSG]      = "sending message",
    [STATUS_RECEIVING_FD]     = "receiving message",
    [STATUS_SENDING_FD]       = "sending message"
};

const char * const SocketClientStateMachine::kCmdStr[] = {
    [CMD_CONNECT_SERVER] = "connect server",
    [CMD_RECEIVE_MSG]    = "receive message",
    [CMD_SEND_MSG]       = "send message",
    [CMD_RECEIVE_FD]     = "receive fd",
    [CMD_SEND_FD]        = "send fd",
};

const char *SocketClientStateMachine::stateName(
    SocketClientStateMachine::status state)
{
    return kStateStr[
        (state < 0 || state > STATUS_SENDING_FD) ?
        STATUS_SENDING_FD : state];
}

const char *SocketClientStateMachine::cmdName(
    SocketClientStateMachine::cmd_type type)
{
    return kCmdStr[
        (type < 0 || type > CMD_SEND_FD) ?
        CMD_SEND_FD : type];
}

void SocketClientStateMachine::logInvalidCmd(
    SocketClientStateMachine::cmd_type cmd)
{
    LOGE(mModule, "Invalid cmd %s in %s state",
        cmdName(cmd), stateName(mStatus));
}

void SocketClientStateMachine::updateToNewStatus(status state)
{
    mStatus = state;
}

int32_t SocketClientStateMachine::procCmdUninitedState(
    SocketClientStateMachine::cmd_type cmd, void * /*arg*/)
{
    logInvalidCmd(cmd);

    return NOT_INITED;
}

int32_t SocketClientStateMachine::procCmdInitedState(
    SocketClientStateMachine::cmd_type cmd, void * /*arg*/)
{
    int32_t rc = NO_ERROR;

    switch (cmd) {
        case CMD_CONNECT_SERVER: {
            cmd_info info;
            info.cmd = cmd;
            rc = executeOnThread(&info);
            if (SUCCEED(rc)) {
                info.sync.wait();
                rc = info.rc;
                if (SUCCEED(rc)) {
                    updateToNewStatus(STATUS_SERVER_CONNECTED);
                    LOGD(mModule, "Connected to server.");
                } else {
                    LOGE(mModule, "Failed to connect server.");
                }
            }
        }; break;
        default: {
            logInvalidCmd(cmd);
        } break;
    }

    return rc;
}

int32_t SocketClientStateMachine::procCmdServerConnectedState(
    SocketClientStateMachine::cmd_type cmd, void *arg)
{
    int32_t rc = NO_ERROR;

    switch (cmd) {
        case CMD_RECEIVE_MSG: {
            updateToNewStatus(STATUS_RECEIVING_MSG);

            cmd_info info;
            info.cmd = cmd;
            info.u.msg = (msg_info *)arg;
            rc = executeOnThread(&info);
            if (SUCCEED(rc)) {
                info.sync.wait();
                rc = info.rc;
                if (!SUCCEED(rc)) {
                    LOGE(mModule, "Failed to receive msg from server.");
                }
            }
            updateToNewStatus(STATUS_SERVER_CONNECTED);
        }; break;
        case CMD_SEND_MSG: {
            updateToNewStatus(STATUS_SENDING_MSG);

            cmd_info info;
            info.cmd = cmd;
            info.u.msg = (msg_info *)arg;
            rc = executeOnThread(&info);
            if (SUCCEED(rc)) {
                info.sync.wait();
                rc = info.rc;
                if (!SUCCEED(rc)) {
                    LOGE(mModule, "Failed to send msg to server.");
                }
            }
            updateToNewStatus(STATUS_SERVER_CONNECTED);
        }; break;
        case CMD_RECEIVE_FD: {
            updateToNewStatus(STATUS_RECEIVING_FD);

            cmd_info info;
            info.cmd = cmd;
            info.u.fd = (int32_t *)arg;
            rc = executeOnThread(&info);
            if (SUCCEED(rc)) {
                info.sync.wait();
                rc = info.rc;
                if (!SUCCEED(rc)) {
                    LOGE(mModule, "Failed to receive fd from server.");
                }
            }
            updateToNewStatus(STATUS_SERVER_CONNECTED);
        }; break;
        case CMD_SEND_FD: {
            updateToNewStatus(STATUS_SENDING_FD);

            cmd_info info;
            info.cmd = cmd;
            info.u.fd = (int32_t *)arg;
            rc = executeOnThread(&info);
            if (SUCCEED(rc)) {
                info.sync.wait();
                rc = info.rc;
                if (!SUCCEED(rc)) {
                    LOGE(mModule, "Failed to send fd to server.");
                }
            }
            updateToNewStatus(STATUS_SERVER_CONNECTED);
        }; break;
        default: {
            logInvalidCmd(cmd);
        } break;
    }

    return rc;
}

int32_t SocketClientStateMachine::procCmdReceivingMsgState(
    SocketClientStateMachine::cmd_type cmd, void *arg)
{
    int32_t rc = NO_ERROR;

    switch (cmd) {
        case CMD_SEND_MSG: {
            updateToNewStatus(STATUS_SENDING_MSG);

            cmd_info info;
            info.cmd = cmd;
            info.u.msg = (msg_info *)arg;
            rc = executeOnThread(&info);
            if (SUCCEED(rc)) {
                info.sync.wait();
                rc = info.rc;
                if (!SUCCEED(rc)) {
                    LOGE(mModule, "Failed to send msg to server.");
                }
            }
            updateToNewStatus(STATUS_SERVER_CONNECTED);
        }; break;
        case CMD_SEND_FD: {
            updateToNewStatus(STATUS_SENDING_FD);

            cmd_info info;
            info.cmd = cmd;
            info.u.fd = (int32_t *)arg;
            rc = executeOnThread(&info);
            if (SUCCEED(rc)) {
                info.sync.wait();
                rc = info.rc;
                if (!SUCCEED(rc)) {
                    LOGE(mModule, "Failed to send fd to server.");
                }
            }
            updateToNewStatus(STATUS_SERVER_CONNECTED);
        }; break;
        default: {
            logInvalidCmd(cmd);
        } break;
    }

    return rc;
}

int32_t SocketClientStateMachine::procCmdSendingMsgState(
    SocketClientStateMachine::cmd_type cmd, void *arg)
{
    int32_t rc = NO_ERROR;

    switch (cmd) {
        case CMD_RECEIVE_MSG: {
            updateToNewStatus(STATUS_RECEIVING_MSG);

            cmd_info info;
            info.cmd = cmd;
            info.u.msg = (msg_info *)arg;
            rc = executeOnThread(&info);
            if (SUCCEED(rc)) {
                info.sync.wait();
                rc = info.rc;
                if (!SUCCEED(rc)) {
                    LOGE(mModule, "Failed to receive msg from server.");
                }
            }
            updateToNewStatus(STATUS_SERVER_CONNECTED);
        }; break;
        case CMD_RECEIVE_FD: {
            updateToNewStatus(STATUS_RECEIVING_FD);

            cmd_info info;
            info.cmd = cmd;
            info.u.fd = (int32_t *)arg;
            rc = executeOnThread(&info);
            if (SUCCEED(rc)) {
                info.sync.wait();
                rc = info.rc;
                if (!SUCCEED(rc)) {
                    LOGE(mModule, "Failed to receive fd from server.");
                }
            }
            updateToNewStatus(STATUS_SERVER_CONNECTED);
        }; break;
        default: {
            logInvalidCmd(cmd);
        } break;
    }

    return rc;
}

int32_t SocketClientStateMachine::procCmdReceivingFdState(
    SocketClientStateMachine::cmd_type cmd, void *arg)
{
    return procCmdReceivingMsgState(cmd, arg);
}

int32_t SocketClientStateMachine::procCmdSendingFdState(
    SocketClientStateMachine::cmd_type cmd, void *arg)
{
    return procCmdSendingMsgState(cmd, arg);
}

};
