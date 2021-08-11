//
// Latest edit by TeeKee on 2017/7/23.
//

/*UTIL_H为预处理变量，防止头文件中变量定义的重复包含；
  util是utiliy的缩写；“H”指代“header”*/
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

/*该函数的作用时是从filename所指向的配置文件中读取根目录、
  端口和线程数等信息到conf所指向的结构体中*/
int read_conf(char* filename, tk_conf_t* conf);
/*SIGPIPE写至读进程已关闭的管道时，默认行为是终止，现该函数现将其都动作改为忽略*/
void handle_for_sigpipe();
/*该函数创建套接字、设置套接字选项、绑定地址，并使绑定地址的套接字处于监听状态*/
int socket_bind_listen(int port);
/*该函数将套接字设置为非阻塞式I/O型，UNPv1第183面有详细解释*/
int make_socket_non_blocking(int fd);
void accept_connection(int listen_fd, int epoll_fd, char* path);

#endif