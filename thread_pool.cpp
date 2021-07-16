#include "thread_pool.hpp"
//mutiple threads programming
//note that lock must be used if shared sources is used in your function segement
/**************
 * create a new thread pool
 * para:
 *      min_num:minimum thread number
 *      max_num:maximum thread number
 *        cap  :capicity of the task queue
 * output:
 *      a pointer points to a new threadpool
-*************/
threadpool_t* threadpool_create(int min_num,int max_num,int cap){
    threadpool_t* newpool = (threadpool_t*)malloc(sizeof(threadpool_t));
    if(!newpool) return NULL;
    //taskqueue init
    newpool->taskQ = (task_t*)malloc(sizeof(task_t)*cap);
    if(!newpool->taskQ) return NULL;
    memset(newpool->taskQ,0,sizeof(task_t)*cap);
    newpool->QueueCapicity = cap;
    newpool->head = 0;
    newpool->rear = 0;
    newpool->Qsize = 0;
    //thread init
    newpool->max_thread = max_num;
    newpool->min_thread = min_num;
    newpool->busy_thread = 0;
    newpool->live_thread = min_num;
    newpool->workers = (pthread_t*)malloc(sizeof(pthread_t)*max_num);
    memset(newpool->workers,0,sizeof(pthread_t)*max_num);
    if(!newpool->workers) return NULL;
    //mutex and cv init
    pthread_cond_init(&newpool->notfull,NULL);
    pthread_cond_init(&newpool->notEmpty,NULL);
    pthread_mutex_init(&newpool->queuemutex,NULL);
    pthread_mutex_init(&newpool->threadmutex,NULL);
    for(int i=0;i<min_num;i++){
        pthread_create(&newpool->workers[i],NULL,work,(void*)newpool);
    }
    pthread_create(&newpool->manager,NULL,manage,(void*)newpool);
    //finish init
    newpool->shutdown = 0;
    newpool->finished_task = 0;
    return newpool;
}

/**************
 * destroy an existed thread pool
 * para:
 *      pool:threadpool pointer
 * output:
 *      status
-*************/
int threadpool_destroy(threadpool_t* pool){
    int i;
    if(pool==NULL) return -1;
    pool->shutdown = true;
    pthread_join(pool->manager,NULL);
    for(i=0;i<pool->live_thread;i++){
        pthread_cond_broadcast(&pool->notEmpty);

    }
    for(i=0;i<pool->live_thread;i++){
        pthread_join(pool->workers[i],NULL);
    }
    threadpool_free(pool);
    return 1;
}

int threadpool_free(threadpool_t* pool){
    if(!pool){
        printf("NO THREAD POOL");
        return -1;
    }
    if(pool->taskQ) free(pool->taskQ);
    if(pool->workers){
        free(pool->workers);
        pthread_mutex_lock(&pool->threadmutex);
        pthread_mutex_destroy(&pool->threadmutex);
        pthread_mutex_lock(&pool->queuemutex);
        pthread_mutex_destroy(&pool->queuemutex);
        pthread_cond_destroy(&pool->notEmpty);
        pthread_cond_destroy(&pool->notfull);
    }
    free(pool);
    pool = NULL;
    return 1;
}

/**************
 * add a new task to the taskqueue of a given threadpool
 * para:
 *      pool:threadpool pointer
 *      newtask:a new task
 * output:
 *      status
-*************/
int add_task(threadpool_t* pool,task_t* newtask){
    pthread_mutex_lock(&pool->queuemutex);
    while(pool->Qsize>=pool->QueueCapicity&&(!pool->shutdown)){ 
        pthread_cond_wait(&pool->notfull,&pool->queuemutex);
    }
    if(pool->shutdown){
        pthread_mutex_unlock(&pool->queuemutex);
        return -1;
    }
    //clear args memory i dont know why 
    //avoid unknown pointer
    if(pool->taskQ[pool->rear].args!=NULL){
        free(pool->taskQ[pool->rear].args);
        pool->taskQ[pool->rear].args = NULL;
    }

    pool->taskQ[pool->rear].function = newtask->function;
    pool->taskQ[pool->rear].args = newtask->args;
    pool->rear = (pool->rear+1)%pool->QueueCapicity;
    pool->Qsize++;
    
    pthread_cond_signal(&pool->notEmpty);
    pthread_mutex_unlock(&pool->queuemutex);
    return 1;
}

//manage thread function
void* manage(void* args){
    int i;
    threadpool_t *pool = (threadpool_t*)args;
    while(!pool->shutdown){
        sleep(DEFAULT_WAIT_TIME);
        pthread_mutex_lock(&pool->queuemutex);
        int qsize = pool->Qsize;
        int live_num = pool->live_thread;
        pthread_mutex_unlock(&pool->queuemutex);
        pthread_mutex_lock(&pool->threadmutex);
        int busy_num = pool->busy_thread;
        pthread_mutex_unlock(&pool->threadmutex);
        printf("NOW busy:%d live:%d!!!\n",busy_num,live_num);

        int add = 0;
        if(qsize>MAX_WAIT_TASK&&live_num<=pool->max_thread){
            pthread_mutex_lock(&pool->queuemutex);
            for(i=0;i<pool->live_thread&&add<DEFAULT_ADD_THREAD&&pool->live_thread<pool->max_thread;i++){
                if(pool->workers[i]==0||is_thread_alive(pool->workers[i])){//==0:not used  is thread alive:idle thread
                    pthread_create(&pool->workers[i],NULL,work,(void*)pool);
                    add++;
                    pool->live_thread++;

                }
            }
            pthread_mutex_unlock(&pool->queuemutex);
        }
        pthread_mutex_lock(&pool->threadmutex);
        printf("%d tasks has been finished\n",pool->finished_task);
        pthread_mutex_unlock(&pool->threadmutex);
    }
    return NULL;
}
//worker thread function
void* work(void* args){
    threadpool_t* pool = (threadpool_t*)args;
    task_t fronttask;
    while(true){
        pthread_mutex_lock(&pool->queuemutex);
        while((pool->Qsize==0)&&(!pool->shutdown)){//wait for a new task
            printf("thread 0x%x is now waiting\n",(unsigned int)pthread_self());
            pthread_cond_wait(&pool->notEmpty,&pool->queuemutex);
            if(pool->live_thread>pool->min_thread){
                printf("thread 0x%x is now exiting\n",(unsigned int)pthread_self());
                pool->live_thread--;
                pthread_mutex_unlock(&pool->queuemutex);
                pthread_exit(NULL);
            }
        }
        if(pool->shutdown){
            pthread_mutex_unlock(&pool->queuemutex);
            printf("thread 0x%x is now exiting\n",pthread_self());
            pthread_exit(NULL);
        }

        fronttask.function = pool->taskQ[pool->head].function;
        fronttask.args = pool->taskQ[pool->head].args;
        pool->head = (pool->head+1)%pool->QueueCapicity;
        pool->Qsize--;

        pthread_cond_broadcast(&pool->notfull);
        pthread_mutex_unlock(&pool->queuemutex);

        printf("thread 0x%x is now busy\n",(unsigned int)pthread_self());
        pthread_mutex_lock(&pool->threadmutex);
        pool->busy_thread++;
        pthread_mutex_unlock(&pool->threadmutex);

        fronttask.function(fronttask.args);
        printf("thread 0x%x finishes its task\n",pthread_self());
        pthread_mutex_lock(&pool->threadmutex);
        pool->busy_thread--;
        pool->finished_task++;
        pthread_mutex_unlock(&pool->threadmutex);
    }
    pthread_exit(NULL);
}


bool is_thread_alive(pthread_t pid){
    int kill_rc = pthread_kill(pid,0);
    if(kill_rc == 3) return false;
    return true;
}
