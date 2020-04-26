#ifndef THREADPOOL
#define THREADPOOL
#include <pthread.h>

/*定义一些出错的信息返回值*/
const int THREADPOOL_INVALID = -1;
const int THREADPOOL_SHUTDOWN = -2;

/*线程池最大线程数量*/
const int MAX_THREADS = 1024;
/*消息队列最大数量*/
const int MAX_QUEUE = 65535;

typedef enum{
    immediate_shutdown = 1,
    graceful_shutdown  = 2
}threadpool_shutdown_t;


struct threadpool_t
{
    pthread_mutex_t lock;
    pthread_cond_t notify;
    pthread_t *thread;
    threadpool_task_t *queue;
    int thread_count;
    int queue_size;
    int head;
    int tail;
    int count;
    int shutdown;
    int started;

};

threadpool_t *threadpool_create(int thread_coutn, int queue_size, int flags);
int threadpool_add(threadpool_t *pool, void (*function)(void *), void *argument, int flags);
int threadpool_destroy(threadpool_t *pool, int flags);
int threadpool_free(threadpool_t *pool);
static void *threadpool_thread(void *threadpool)
#endif