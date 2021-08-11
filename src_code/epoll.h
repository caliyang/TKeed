//
// Latest edit by TeeKee on 2017/7/23.
//

#ifndef EPOLL_H
#define EPOLL_H

#include <sys/epoll.h>
#include "http.h"
#include "threadpool.h"

#define MAXEVENTS 1024

/*创建一个epoll句柄，并为events结构体数组分配内存*/
int tk_epoll_create(int flags);
/*注册新描述符*/
int tk_epoll_add(int epoll_fd, int fd, tk_http_request_t* request, int events);
/*修改描述符状态*/
int tk_epoll_mod(int epoll_fd, int fd, tk_http_request_t* request, int events);
/*从epoll中删除描述符*/
int tk_epoll_del(int epoll_fd, int fd, tk_http_request_t* request, int events);
/*返回活跃事件数*/
int tk_epoll_wait(int epoll_fd, struct epoll_event *events, int max_events, int timeout);
/*分发处理函数*/
void tk_handle_events(int epoll_fd, int listen_fd, struct epoll_event* events,
                      int events_num, char* path, tk_threadpool_t* tp);

#endif
