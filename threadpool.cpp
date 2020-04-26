#include "threadpool.h"

threadpool_t *threadpool_create(int thread_count, int queue_size, int flags)
{
    threadpool_t *pool;
    int i;
    do
    {
        if(thread_count <= 0 || thread_count > MAX_THREADS
          || queue_size <= 0 || queue_size > MAX_QUEUE)
          {
              return null;
          }
        
        if((pool = (threadpool_t)))
    }
}