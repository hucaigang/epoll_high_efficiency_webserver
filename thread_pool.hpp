#ifndef __THREAD_POOL_HPP__
#define __THREAD_POOL_HPP__
#include <pthread.h>
#include <malloc.h>
#include <memory.h>
#include <unistd.h>
#include <signal.h>

#define DEFAULT_WAIT_TIME 5 // DEFAULT SLEEP TIME
#define DEFAULT_ADD_THREAD 2 //DEFAULT SET OF ADDING THREAD ONCE
#define MAX_WAIT_TASK 20 //MAX NUMBER OF WAITING TASK
typedef struct task{
    void (*function)(void*);
    void* args;
};

typedef struct threadpool{
    task* taskQ;
    int QueueCapicity;
    int Qsize;
    int head; //head
    int rear; //rear

    int max_thread;
    int min_thread;
    int busy_thread;
    int live_thread;

    pthread_cond_t notfull; //queue is not full
    pthread_cond_t notEmpty; //queue is not empty
    pthread_mutex_t queuemutex; //queue mutex
    pthread_mutex_t threadmutex; //pool mutex

    pthread_t manager;
    pthread_t* workers;

    int shutdown;
    int finished_task;
};
typedef struct threadpool threadpool_t;
typedef struct task task_t;
/**************
 * create a new thread pool
 * para:
 *      min_num:minimum thread number
 *      max_num:maximum thread number
 *        cap  :capicity of the task queue
 * output:
 *      a pointer points to a new threadpool
-*************/
threadpool_t* threadpool_create(int min_num,int max_num,int cap);

/**************
 * destroy an existed thread pool
 * para:
 *      pool:threadpool pointer
 * output:
 *      status
-*************/
int threadpool_destroy(threadpool_t* pool);

/**************
 * add a new task to the taskqueue of a given threadpool
 * para:
 *      pool:threadpool pointer
 *      newtask:a new task
 * output:
 *      status
-*************/
int add_task(threadpool_t* pool,task_t* newtask);

/**************
 * get an task from the taskqueue of a given threadpool
 * para:
 *      pool:threadpool pointer
 * output:
 *      status
-*************/

//manage thread function
void* manage(void* args);
//worker thread function
void* work(void* args);

bool is_thread_alive(pthread_t pid);
int threadpool_free(threadpool_t* pool);
#endif