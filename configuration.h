#ifndef _SIRIUS_CONFIGURATION_H_
#define _SIRIUS_CONFIGURATION_H_

namespace sirius {

#define SERVER_SOCKET_PATH         "/data/misc/camera/"
#define SERVER_SOCKET_NAME         "sirius_socket"
#define SOCKET_DATA_MAX_LEN        128 // byte
#define SERVER_PULL_CLIENT_TIMEOUT 100 // ms
#define SERVER_POLL_DATA_TIMEOUT   200
#define CLIENT_POLL_DATA_TIMEOUT   200
#define CLIENT_CONNECT_RETRY_TIME  100
#define CLIENT_CONNECT_NOTIFIER_TIME 5

#define MAX_CLIENT_ALLOWED         20

#define SOCKET_CLIENT_GREETING_STR "Hello, Sirius."
#define SOCKET_CLIENT_REPLY_STR    "Done."

#define SOCKET_CLIENT_CONNECT_TYPE    "Request type"
#define SOCKET_SERVER_SHARE_STR       "Mem share, type"
#define SOCKET_CLIENT_REPLY_SHARE_STR "Mem share, OK."
#define SOCKET_CLIENT_REPLY_FD_STR    "Done, fd"
#define SOCKET_SERVER_SHARE_DONE      "Mem share done."
#define SOCKET_CLIENT_NOTIFIER_STR    "Ready:"
#define SOCKET_EVENT_REQUEST_FORMAT   "EVT:"

#define REQUEST_HANDLER_MAX_MEMORY_NUM 10

};

#endif
