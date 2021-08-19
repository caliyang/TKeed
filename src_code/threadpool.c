//
// Latest edit by TeeKee on 2017/7/26.
//


#include "threadpool.h"

/* 释放线程池中的线程数组和task链表 */
static int threadpool_free(tk_threadpool_t *pool);
/* 线程处理函数，包含任务处理函数 */
static void* threadpool_worker(void *arg); 

// 释放线程池
/* 释放线程池中的线程数组和task链表 */
/* 其他几个数据成员是如何销毁的，02？？？其他几个变量是存放在栈上的，系统会自动销毁，不需要考虑；
   直接free pool行吗，02？？？不行，thread指针和head指针所指向的对象是建立在堆上的，需要另外专门销毁 */
int threadpool_free(tk_threadpool_t *pool){
    /* pool->started > 0 这一判断对应于执行线程的个数大于 0，也就是在调用threadpool_free前未释放线程资源 */
    if(pool == NULL || pool->started > 0)
        return -1;

    // 释放线程数组
    /*线程池pool是由malloc分配的，因此里面的指针成员指向的内存是由free()函数释放*/
    if(pool->threads)
        free(pool->threads); // #include <stdlib.h>

    // 逐节点销毁task链表
    /*从头节点一直 free 到尾节点*/
    tk_task_t *old;
    while(pool->head->next){
        old = pool->head->next;
        pool->head->next = pool->head->next->next;
        free(old); // #include <stdlib.h>
    }
    return 0;
}

/* 线程处理函数，包含任务处理函数 */
void *threadpool_worker(void *arg){
    if(arg == NULL)
        return NULL;
    
    /* 将void指针转换为tk_threadpool_t指针 */
    tk_threadpool_t *pool = (tk_threadpool_t *)arg;
    tk_task_t *task;
    while(1){
        // 对线程池上锁
        pthread_mutex_lock(&(pool->lock));

        /* shutdown：0-未停机模式，1-立即停机模式，2-平滑停机模式 */
        // 没有task且未停机则阻塞
        /* 用while循环防止虚假唤醒 */
        while((pool->queue_size == 0) && !(pool->shutdown))
            pthread_cond_wait(&(pool->cond), &(pool->lock));
        
        // 立即停机模式、平滑停机且没有未完成任务则退出
        /* 工作线程立即停机退出 */
        if(pool->shutdown == immediate_shutdown)
            break;
        /* 平滑停机且没有未完成任务时退出 */
        else if((pool->shutdown == graceful_shutdown) && (pool->queue_size == 0))
            break;

        // 得到第一个task
        /* 将头节点拷贝赋值给task节点 */
        task = pool->head->next;
        // 没有task则开锁并进行下一次循环
        /* while循环可以保证task不为null了吧，02？？？while循环可以保证有task，
           但task本身是可以由null来充当的，因此需要判断下 */
        if(task == NULL){
            pthread_mutex_unlock(&(pool->lock));
            continue;
        }

        // 存在task则取走并开锁
        /* 既然是双向链表，应该需要task->next->prev = pool->next吧，02？？？
           不需要，tk_task结构中无prev指针, 不要和list_head结构想混了 */
        pool->head->next = task->next;
        pool->queue_size--;
        /* 虽然pool->head->next = task->next，但是之前task = pool->head->next中的 
           task还在，它依然指向原先的pool->head->next节点 */
        pthread_mutex_unlock(&(pool->lock));

        // 设置task中func参数
        /* 调用函数指针fun，执行任务处理函数 */
        (*(task->func))(task->arg);
        /* 任务处理函数返回，task结束，释放task节点 */
        free(task);
    }
    
    /* 工作线程的个数减1 */
    pool->started--;
    pthread_mutex_unlock(&(pool->lock));
    /* #define NULL ((void *)0)，线程的退出码为0 */
    pthread_exit(NULL);
    /* pthread_exit() 函数已经可以让工作线程退出，return是不是有些多余，02？？？ */
    return NULL;
}

// 释放线程资源
/* 释放的线程资源包括互斥量和条件变量的释放与销毁，以及回收线程资源 */
int threadpool_destory(tk_threadpool_t *pool, int graceful){
    if(pool == NULL)
        /*枚举元素都是宏定义的常数*/
        return tk_tp_invalid;
    /* 不加锁的话，后续if(pool->shutdown)的判断可能会报错 */
    if(pthread_mutex_lock(&(pool->lock)) != 0)
        return tk_tp_lock_fail;

    int err = 0;
    /* 当执行一段代码到中间，想跳过剩下一半时，借助do{...} while(0);和break就能达到目的；
       while(0)意味着循环体只执行一次 */
    do{
        /* pool->shutdown值为1或2时进入，为什么需要这个判断，02？？？
           shutdown的值由0变为1或2，应该是threadpool_destory的结果，在未调用前，是不应该出现的 */
        if(pool->shutdown){
            err = tk_tp_already_shutdown;
            break;
        }

        /* graceful是该函数接受的参数 */
        pool->shutdown = (graceful) ? graceful_shutdown : immediate_shutdown;
        /* 通过pthread_cond_broadcast，让剩余工作线程退出 */
        if(pthread_cond_broadcast(&(pool->cond)) != 0){
            err = tk_tp_cond_broadcast;
            break;
        }

        if(pthread_mutex_unlock(&(pool->lock)) != 0){
            err = tk_tp_lock_fail;
            break;
        }

        // 回收每个线程资源
        for(int i = 0; i < pool->thread_count; i++){
            if(pthread_join(pool->threads[i], NULL) != 0){
                err = tk_tp_thread_fail;
            }
        }
    }while(0);

    /* 释放内存前销毁互斥量和条件变量 */
    if(!err){
        pthread_mutex_destroy(&(pool->lock));
        pthread_cond_destroy(&(pool->cond));
        threadpool_free(pool);
    }
    return err;
}

// 初始化线程池
/* 动态分配并初始化线程池 */
tk_threadpool_t *threadpool_init(int thread_num){
    // 分配线程池
    /* 定义线程池结构体指针，分配线程池结构体内存，
       再将malloc返回的指针拷贝赋值给之前定义的线程池结构体指针 */
    tk_threadpool_t* pool;
    if((pool = (tk_threadpool_t *)malloc(sizeof(tk_threadpool_t))) == NULL) // #include <stdlib.h>
        goto err;

    // threads指针指向线程数组（存放tid），数组大小即为线程数
    pool->thread_count = 0;  /* 线程数 */
    pool->queue_size = 0; /* 任务链表长 */
    pool->shutdown = 0; /* 关机模式 */
    pool->started = 0;
    /* 动态分配线程数组（存放tid），数组大小即为线程数 */
    pool->threads = (pthread_t*)malloc(sizeof(pthread_t) * thread_num);
    
    // 分配并初始化task头节点
    /* 动态分配并初始化task链表中的头节点 */
    pool->head = (tk_task_t*)malloc(sizeof(tk_task_t));
    if((pool->threads == NULL) || (pool->head == NULL))
        goto err;
    pool->head->func = NULL;
    pool->head->arg = NULL;
    pool->head->next = NULL;

    // 初始化锁
    if(pthread_mutex_init(&(pool->lock), NULL) != 0) // #include <pthread.h>
        goto err;

    // 初始化条件变量
    if(pthread_cond_init(&(pool->cond), NULL) != 0)  // #include <pthread.h>
        goto err;

    // 创建线程
    for(int i = 0; i < thread_num; ++i){
        /* pool是传给任务处理函数的参数 */
        if(pthread_create(&(pool->threads[i]), NULL, threadpool_worker, (void*)pool) != 0){ // #include <pthread.h>
            threadpool_destory(pool, 0);
            return NULL;
        }
        pool->thread_count++;
        pool->started++;
    }
    return pool;

/* 若指针pool已分配内存，则先释放在返回NULL，若没有分配内存，则直接返回NULL */
/* 如果是在初始化cond时转跳到这里，那是否需要在free pool前，先destory掉lock，02？？？ */
err:
    if(pool)
        threadpool_free(pool);
    return NULL;
}

/* 向线程池的task链表中添加节点，位于头节点后 */
int threadpool_add(tk_threadpool_t* pool, void (*func)(void *), void *arg){
    /* 有必要定义rc吗，02？？？ */
    int rc, err = 0;
    if(pool == NULL || func == NULL)
        return -1;
    
    /* 不加锁的话，当两个线程同时执行到task->next = pool->head->next;时，
       先执行pool->head->next = task;语句的线程的任务添加会遗漏掉，因此需要加锁，按序添加task节点 */
    if(pthread_mutex_lock(&(pool->lock)) != 0)
        return -1;

    // 已设置关机
    if(pool->shutdown){
        err = tk_tp_already_shutdown;
        goto out;
    }

    // 新建task并注册信息
    /* 分配并初始化task节点 */
    tk_task_t *task = (tk_task_t *)malloc(sizeof(tk_task_t));
    if(task == NULL)
        goto out;
    /* 任务处理函数 */
    task->func = func;
    /* 任务处理函数的参数 */
    task->arg = arg;

    // 新task节点在head处插入
    /* 向链表插入或删除节点的顺序：“先加后改再删” */
    task->next = pool->head->next;
    pool->head->next = task;
    pool->queue_size++;

    rc = pthread_cond_signal(&(pool->cond));

out:
    if(pthread_mutex_unlock(&pool->lock) != 0)
        return -1;
    return err;
}
