//
// Latest edit by TeeKee on 2017/7/23.
//


#include <sys/time.h>
#include "timer.h"

/* timer为优先队列结构体 */
tk_pq_t tk_timer;
/* current_msec为当前时间，单位为ms */
size_t tk_current_msec;

/* 比较两个计时器的超时时间，左边小于右边则返回1，否则返回0 */
int timer_comp(void* ti, void* tj){
    tk_timer_t* timeri = (tk_timer_t*)ti;
    tk_timer_t* timerj = (tk_timer_t*)tj;
    return (timeri->key < timerj->key) ? 1 : 0;
}

// 获取当前时间
/* 更新并记录当前时间，将当前时间用微秒表示，并存储在tk_current_msec中 */
void tk_time_update(){
    /* 通过gettimeofday函数将当前时间表示为秒和微秒并存储在timeval结构中 */
    struct timeval tv;
    int rc = gettimeofday(&tv, NULL); // #include <sys/time.h>
    tk_current_msec = ((tv.tv_sec * 1000) + (tv.tv_usec / 1000)); 
}

/* 初始化timer优先队列，比较函数置为timer_comp，优先队列里面的成员为 time_t 结构体 */
int tk_timer_init(){
    // 建立连接后立即初始化
    // 初始优先队列大小TK_PQ_DEFAULT_SIZE = 10
    /* tk_pq_init()函数最后一个参数的含义为优先队列节点的个数，不包括头节点 */
    int rc = tk_pq_init(&tk_timer, timer_comp, TK_PQ_DEFAULT_SIZE);

    // 更新当前时间
    tk_time_update();
    return 0;
}

/* 超时查找函数：该函数返回优先队列中最早时间和当前时间之差，以此来确定队列中是否有超时的计时器 */
/* 如果优先队列中无节点，那么返回的time为未定义值，这样处理不好吧。可以这样：
   46 int time = 0;
   69 这一句可以去掉； */
int tk_find_timer(){
    int time;
    // 返回队列中最早时间和当前时间之差
    while(!tk_pq_is_empty(&tk_timer)){
        // 更新当前时间
        tk_time_update();
        // timer_node指向最小的时间
        /* 最小的时间，即key为最小的timer_t结构体 */
        /* 将节点的void*转换为tk_timer_t* */
        tk_timer_t* timer_node = (tk_timer_t*)tk_pq_min(&tk_timer);
        // 如果已删则释放此节点（tk_del_timer只置位不删除）
        /* 在查找的时候删除，而不是tk_del_timer立即删除或专门用某个程序删除，前者效率会更高效 */
        /* 检查该节点是否标记为“删除” */
        if(timer_node->deleted){
            /* 删除标记为delete的优先队列节点 */
            int rc = tk_pq_delmin(&tk_timer);
            /* 释放为该优先队列节点分配的空间 */
            free(timer_node);
            continue;
        }
        // 此时timer_node为时间最小节点，time为优先队列里最小时间减去当前时间
        time = (int)(timer_node->key - tk_current_msec);
        time = (time > 0) ? time : 0;
        break;
    }

    return time;
}

/* 超时检查函数 */
/* 检查并处理优先队列节点等待时间超时的request请求 */
/* tk_handle_expire_timers会检查、处理并删除超时优先队列节点，但tk_del_timer不会 */
void tk_handle_expire_timers(){
    while(!tk_pq_is_empty(&tk_timer)){
        // 更新当前时间
        tk_time_update();
        tk_timer_t* timer_node = (tk_timer_t*)tk_pq_min(&tk_timer);
        // 如果已删则释放此节点
        if(timer_node->deleted){
            /* 删除标记为 delete 的优先队列节点 */
            int rc = tk_pq_delmin(&tk_timer);
            /* 释放该优先队列节点所分配的空间 */
            free(timer_node);
            continue;
        }
        // 最早入队列节点超时时间大于当前时间（未超时）
        // 结束超时检查，顺带删了下标记为删除的节点
        /* 通过上面的 if(timer_node->deleted) 删除标记为删除的节点 */
        if(timer_node->key > tk_current_msec){
            return;
        }
        // 出现了没被删但是超时的情况，调用handler处理
        /* 处理对应的request请求 */
        if(timer_node->handler){
            timer_node->handler(timer_node->request);
        }
        /* 处理完后，删除相应优先队列节点 */
        int rc = tk_pq_delmin(&tk_timer); 
        /* 释放该优先队列节点所分配的空间 */
        free(timer_node);
    }
}

/* 将新创建的timer_t节点加入http_request_t结构体和插入timer优先队列中 */
void tk_add_timer(tk_http_request_t* request, size_t timeout, timer_handler_pt handler){
    tk_time_update();
    // 申请新的tk_timer_t节点，并加入到tk_http_request_t的timer下
    /* 为优先队列节点分配空间 */
    tk_timer_t* timer_node = (tk_timer_t*)malloc(sizeof(tk_timer_t));
    request->timer = timer_node;
    /* 初始化timer_node指向的tk_timer_t节点 */
    // 加入时，设置超时阈值，删除信息等
    timer_node->key = tk_current_msec + timeout;
    timer_node->deleted = 0;
    timer_node->handler = handler;
    // 注意需要在tk_timer_t节点中反向设置指向对应resquest的指针
    timer_node->request = request;
    // 将新节点插入优先队列
    /* 将新创建的tk_timer_t节点插入timer优先队列中 */
    int rc = tk_pq_insert(&tk_timer, timer_node);
}

/* 将http_request_t中的时间戳结构中的deleted成员置为1 */
void tk_del_timer(tk_http_request_t* request) {
    tk_time_update();
    tk_timer_t* timer_node = request->timer;
    // 惰性删除
    // 标记为已删，在find_timer和handle_expire_timers检查队列时会删除
    timer_node->deleted = 1;
}