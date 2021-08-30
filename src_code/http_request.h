//
// Latest edit by TeeKee on 2017/7/23.
//

#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "util.h"
#include "list.h"

#define TK_AGAIN EAGAIN

/* 请求方法中的字符ch满足(ch < 'A' || ch > 'Z') && ch != '_'时，返回该宏定义 */
#define TK_HTTP_PARSE_INVALID_METHOD        10 
/* 无效URL地址，URL首字符不是'/'或' '（' '会被过滤掉）时，返回该宏定义 */
#define TK_HTTP_PARSE_INVALID_REQUEST       11
#define TK_HTTP_PARSE_INVALID_HEADER        12

/* 请求方法中的字符ch满足'A' <= ch <= 'Z' || ch == '_'时，返回该宏定义 */
#define TK_HTTP_UNKNOWN                     0x0001
#define TK_HTTP_GET                         0x0002
#define TK_HTTP_HEAD                        0x0004
#define TK_HTTP_POST                        0x0008

#define TK_HTTP_OK                          200
#define TK_HTTP_NOT_MODIFIED                304
#define TK_HTTP_NOT_FOUND                   404
#define MAX_BUF 8124

/* 请求信息结构 */
typedef struct tk_http_request{
    char* root; // 配置目录
    int fd; // 描述符（监听、连接）
    int epoll_fd; // epoll描述符
    char buff[MAX_BUF]; // 用户缓冲
    /* 累计已经解析缓冲区的字节数，从数组下标来看，也是该次http请求解析的起始位置 */
    size_t pos;
    /* 累计已经写入缓冲区的字节数，从数组下标来看，也是该次http请求解析的终止位置 */
    size_t last;
    int state; // 请求头解析状态
    /* 在初始化函数tk_init_request_t中，除了buff数组，以上变量都已初始化 */

    /* 指向请求方法中的首个字符，比如GET中的'G' */
    void* request_start;
    /* 指向请求方法后的空格字符 */
    void* method_end;
    /* 仅支持http 1.0，因此请求方法只有GET、POST和HEAD */
    int method; // 请求方法
    /* 指向URL地址的第二个字符，即'/'后的一个字符 */
    void* uri_start;
    /* 指向URL地址后面的空格字符 */
    void* uri_end;
    void* path_start;
    void* path_end;
    void* query_start;
    void* query_end;
    /* http的主版本号 */
    int http_major;
    /* http的副版本号 */
    int http_minor;
    /* 指向请求行末尾的换行标志的第一个字符，对于Windows操作系统为回车符，对于Unix系统为换行符 */
    /* 该服务器不考虑Mac系统 */
    void* request_end;

    struct list_head list; // 存储请求头，list.h中定义了此结构

    void* cur_header_key_start;
    void* cur_header_key_end;
    void* cur_header_value_start;
    void* cur_header_value_end;
    /* 设置http_request结构体的timer成员，该成员是timer优先级队列中的节点  */
    void* timer; // 指向时间戳结构
}tk_http_request_t;

/* 响应头结构 */
typedef struct tk_http_out{
    int fd;
    int keep_alive;
    time_t mtime;
    int modified;
    int status;
}tk_http_out_t;

typedef struct tk_http_header{
    void* key_start;
    void* key_end;
    void* value_start;
    void* value_end;
    struct list_head list;
}tk_http_header_t;

typedef int (*tk_http_header_handler_pt)(tk_http_request_t* request, tk_http_out_t* out, char* data, int len);

typedef struct tk_http_header_handle{
    char* name;
    tk_http_header_handler_pt handler;    // 函数指针
}tk_http_header_handle_t;

/* 全局变量tk_http_headers_in的声明 */
extern tk_http_header_handle_t tk_http_headers_in[]; 

void tk_http_handle_header(tk_http_request_t* request, tk_http_out_t* out);
int tk_http_close_conn(tk_http_request_t* request);
int tk_init_request_t(tk_http_request_t* request, int fd, int epoll_fd, char* path);
int tk_init_out_t(tk_http_out_t* out, int fd);
const char* get_shortmsg_from_status_code(int status_code);

#endif