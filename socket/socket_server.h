#ifndef _SIRIUS_SERVER_H_
#define _SIRIUS_SERVER_H_

#include "server_client_common.h"

namespace sirius {

int32_t start_server(int32_t *socketfd);

int32_t poll_accept(int32_t sockfd, int32_t *clientfd);

int32_t poll_read(int32_t clientfd,
    char *dat, int32_t max_len, int32_t *read_len);

int32_t poll_accept_wait(int32_t sockfd,
    int32_t *clientfd, int32_t *cancel);

int32_t poll_read_wait(int32_t clientfd,
    char *dat, int32_t max_len, int32_t *read_len, int32_t *cancel);

int32_t disconnect_client(int32_t clientfd);

int32_t stop_server(int32_t sockfd);

};

#endif