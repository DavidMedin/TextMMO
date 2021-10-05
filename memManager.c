#include "memManager.h"
extern void Fatal(const char* func,int rv);
union {
    unsigned int size;
    void* mem;
}arg;
int argType = 0;//type is 0->free or 1->allocate or 2->exit (2 to shut clion up)
void AllocatorWorker(void* nothing){
    int rv;
    if((rv=nng_cv_alloc(&condiVar,mtx))!=0){
        Fatal("nng_cv_alloc",rv);
        return;
    }
    while(1){
        printf("waiting for control\n");
        nng_mtx_lock(mtx);
        nng_cv_wait(condiVar);
        if(argType == 0){
            free(arg.mem);
        }else if(argType == 1){
            arg.mem = malloc(arg.size);
        }else if(argType == 2){
            return;
        }else
            printf("%d is not a vaild arg type for the worker thread\n",argType);
    }
}
int StartAllocator(){
    int rv;
    if((rv=nng_mtx_alloc(&mtx))!=0){
        Fatal("Allocator nng_alloc_mtx",rv);
        return 1;
    }
    if((rv=nng_thread_create(&memThread,AllocatorWorker,NULL))!=0){
        Fatal("nng_thread_create",rv);
        return 1;
    }
    return 0;
}
void KillAllocator(){
    nng_thread_destroy(memThread);
    nng_mtx_free(mtx);
    nng_cv_free(condiVar);
}
void* RequestMemory(unsigned int size){
    nng_mtx_lock(mtx);
    argType = 1;
    arg.size = size;
    nng_cv_wake(condiVar);//wake the thread
    nng_mtx_unlock(mtx);//give control to thread
    nng_mtx_lock(mtx);//wait for thread to finish
    nng_mtx_unlock(mtx);//unlock for future use
    return arg.mem;
}
void FreeMemory(void* mem){
    nng_mtx_lock(mtx);
    argType = 0;
    arg.mem = mem;
    nng_cv_wake(condiVar);//wake the thread
    nng_mtx_unlock(mtx);//give control to thread
}
