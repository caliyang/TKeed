//
// Latest edit by TeeKee on 2017/7/23.
//

// 系统头文件
#include <stdio.h>
#include "threadpool.h"
#include "http.h"

#define DEFAULT_CONFIG "tkeed.conf"

/*声明全局变量结构体指针events*/
extern struct epoll_event *events;  
/*conf_file char指针指向"tkeed.conf"字符串*/
char *conf_file = DEFAULT_CONFIG;  
/*typedef将struct tk_conf的别名定义为tk_conf_t的原因*/
tk_conf_t conf;

int main(int argc, char *argv[]){
    // 读取配置文件
    /*用配置文件填充conf结构体*/
    read_conf(conf_file, &conf);

    // 处理SIGPIPE
    /*SIGPIPE写至无读进程的管道，默认行为是终止，现将其都动作改为忽略*/
    handle_for_sigpipe();

    // 初始化套接字开始监听
    /*该函数创建套接字、设置套接字选项、绑定地址，并使绑定地址的套接字处于监听状态*/
    int listen_fd = socket_bind_listen(conf.port);

    // 设置为socket非阻塞
    /*该函数将套接字设置为非阻塞式I/O型，UNPv1第183面有详细解释*/
    int rc = make_socket_non_blocking(listen_fd);

    // 创建epoll并注册监听描述符
    /*创建一个epoll句柄，并为event结构体数组分分配内存*/
    int epoll_fd = tk_epoll_create(0);
    /*为tk_http_request_t结构体分配内存*/
    tk_http_request_t* request = (tk_http_request_t*)malloc(sizeof(tk_http_request_t));
     /*初始化tk_http_request_t结构体*/
    tk_init_request_t(request, listen_fd, epoll_fd, conf.root);
    tk_epoll_add(epoll_fd, listen_fd, request, (EPOLLIN | EPOLLET));

    // 初始化线程池
    tk_threadpool_t *tp = threadpool_init(conf.thread_num);

    // 初始化计时器
    tk_timer_init();

    while(1){
        // 得到最近且未删除时间和当前时间差值（等待时间）
        int time = tk_find_timer();

        // 调用epoll_wait函数，返回接收到事件的数量
        int events_num = tk_epoll_wait(epoll_fd, events, MAXEVENTS, -1);

        // 处理已经超时的请求
        tk_handle_expire_timers();

        // 遍历events数组，根据监听种类及描述符类型分发操作
        tk_handle_events(epoll_fd, listen_fd, events, events_num, conf.root, tp);
    }

    // 回收线程资源
    // threadpool_destroy(tp, graceful_shutdown);
}

