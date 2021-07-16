#include "thread_pool.hpp"
void print(void* args){
    printf("hello\n");
    sleep(1);
}


int main(){
    threadpool_t* a = threadpool_create(10,100,2000);
    task_t* p = (task*)malloc(sizeof(task_t));
    p->args = NULL;
    p->function = print;
    for(int i=0;i<500;i++){
        add_task(a,p);
    }
    while(1);
    threadpool_destroy(a);
    return 0;
}