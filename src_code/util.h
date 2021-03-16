//
// Latest edit by TeeKee on 2017/7/23.
//

/*UTIL_H为预处理变量，防止头文件中变量定义的重复包含；util是utiliy的缩写；“H”指代“header”*/
#ifndef UTIL_H
#define UTIL_H

/*路径名长度*/
#define PATHLEN 128
#define LISTENQ 1024
/*缓冲区长度*/
#define BUFLEN 8192
#define DELIM "="

#define TK_CONF_OK 0
#define TK_CONF_ERROR -1

/*用MIN宏定义来代替min函数*/
#define MIN(a, b) ((a) < (b) ? (a) : (b))

typedef struct tk_conf{
    char root[PATHLEN];
    int port;
    int thread_num;
}tk_conf_t;

int read_conf(char* filename, tk_conf_t* conf);
void handle_for_sigpipe();
int socket_bind_listen(int port);
int make_socket_non_blocking(int fd);
void accept_connection(int listen_fd, int epoll_fd, char* path);

#endif

/*
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include "util.h"
#include "http_request.h"
#include "epoll.h"
*/