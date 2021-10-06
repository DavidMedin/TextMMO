#include "memManager.h"
extern void Fatal(const char* func,int rv);
union {
    unsigned int size;
    void* mem;
}arg;
int argType = 0;//type is 0->free or 1->allocate or 2->exit (2 to shut clion up)
void AllocatorWorker(void* nothing){
    while(1){
        printf("waiting for control\n");
        nng_mtx_lock(startRMMut);
        nng_cv_wait(startRMVar);
        if(argType == 0){
            free(arg.mem);
        }else if(argType == 1){
            arg.mem = malloc(arg.size);
            printf("allocated memory\n");
        }else if(argType == 2){
            return;
        }else
            printf("%d is not a vaild arg type for the worker thread\n",argType);
        nng_mtx_lock(endRMMut);
        nng_mtx_unlock(endRMMut);
        nng_mtx_unlock(startRMMut);
    }
}
int StartAllocator(){
    int rv;
    nng_mtx** muts[3] = {&totalRMMut,&startRMMut,&endRMMut};
    for(int i = 0;i < 3;i++){
        if((rv=nng_mtx_alloc(muts[i]))!=0){
            printf("%d: ",i);
            Fatal("Allocator nng_alloc_mtx",rv);
            return 1;
        }
    }
    if((rv=nng_cv_alloc(&startRMVar,startRMMut))!=0){
        Fatal("nng_cv_alloc",rv);
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
    nng_mtx* muts[3] = {totalRMMut,startRMMut,endRMMut};
    for(int i = 0;i < 3;i++){
        nng_mtx_free(muts[i]);
    }
    nng_cv_free(startRMVar);
}
void* RequestMemory(unsigned int size){
    //nng_mtx_lock(mtx);
    nng_mtx_lock(totalRMMut);
    argType = 1;
    arg.size = size;
    nng_mtx_lock(startRMMut);
    nng_cv_wake(startRMVar);//wake the thread
    nng_mtx_lock(endRMMut);//hold something so the thread can't end till we are ready
    nng_mtx_unlock(startRMMut);//give control to thread
    nng_mtx_unlock(endRMMut);//unlock for future use
    void* mem = arg.mem;
    nng_mtx_unlock(totalRMMut);
    return mem;
}
void FreeMemory(void* mem){
    nng_mtx_lock(totalRMMut);
    argType = 0;
    arg.mem = mem;
    nng_mtx_lock(startRMMut);
    nng_cv_wake(startRMVar);//wake the thread
    nng_mtx_lock(startRMMut);
    nng_mtx_unlock(startRMMut);//give control to thread
    nng_mtx_unlock(endRMMut);
    nng_mtx_unlock(totalRMMut);
}
