#include "vec.h"
#include <stdlib.h>
#include <memory.h>
Vec VecMake(int size,int initCount){
    Vec tmpVec={0};
    tmpVec.size = size;
    if(initCount != 0){
        tmpVec.data = malloc(size*(initCount/VECTOR_ALLOC_SIZE+1)*VECTOR_ALLOC_SIZE);
        tmpVec.size = size;
        tmpVec.count = initCount;
        tmpVec.allocCount = initCount/VECTOR_ALLOC_SIZE+1;
    }
    return tmpVec;
}
void* VecNext(Vec* vec){
    if(++vec->count > vec->allocCount){
        //alloc more mem
        vec->allocCount += VECTOR_ALLOC_SIZE;
        char* tmpData = malloc(vec->allocCount * vec->size);
        memcpy(tmpData,vec->data,vec->allocCount - VECTOR_ALLOC_SIZE);
        free(vec->data);
        vec->data = tmpData;
    }
    return &((char*)vec->data)[vec->count-1];
}
void* VecLast(Vec* vec){
    return &((char*)vec->data)[vec->count-1];
}

void VecDestroy(Vec* vec){
    free(vec->data);
    vec->size = 0;
    vec->count = 0;
    vec->allocCount = 0;
    vec->data = NULL;
}