//
// Latest edit by TeeKee on 2017/7/26.
//

#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>

/*task结构*/
typedef struct tk_task{
    void (*func)(void*);
    void* arg;
    struct tk_task* next;    // 任务链表（下一节点指针）
}tk_task_t;

/* threadpool结构*/
typedef struct threadpool{
    pthread_mutex_t lock;    // 互斥锁；#include <pthreadtypes.h>，包含于#include <pthread.h>
    pthread_cond_t cond;    // 条件变量；#include <pthreadtypes.h>，包含于#include <pthread.h>
    pthread_t *threads;    // 线程；#include <pthreadtypes.h>，包含于#include <pthread.h>
    tk_task_t *head;    // 任务链表（链表中某节点）
    int thread_count;    // 线程数 
    int queue_size;    // 任务链表长
    int shutdown;     // 关机模式
    int started;
}tk_threadpool_t;


/*threadpool_error 枚举类型别名*/
typedef enum{
    tk_tp_invalid = -1,
    tk_tp_lock_fail = -2,
    tk_tp_already_shutdown = -3,
    tk_tp_cond_broadcast = -4,
    tk_tp_thread_fail = -5,
}tk_threadpool_error_t;

/*threadpool_sd 枚举类型别名*/
typedef enum{
    immediate_shutdown = 1,
    graceful_shutdown = 2
}tk_threadpool_sd_t;

tk_threadpool_t* threadpool_init(int thread_num);
int threadpool_add(tk_threadpool_t* pool, void (*func)(void *), void* arg);
int threadpool_destroy(tk_threadpool_t* pool, int gracegul);

#endif
