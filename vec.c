#include "vec.h"
#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
Vec VecMake(int size,int initCount){
    Vec tmpVec={0};
    tmpVec.size = size;
    if(initCount != 0){
        tmpVec.data = malloc(size*(initCount/VECTOR_ALLOC_SIZE+1)*VECTOR_ALLOC_SIZE);
        tmpVec.size = size;
        tmpVec.last = 0;
        tmpVec.allocCount = (((initCount-1)/VECTOR_ALLOC_SIZE)+1)*VECTOR_ALLOC_SIZE;
    }
    return tmpVec;
}
void* VecNext(Vec* vec){
    if((++vec->last) >= vec->allocCount){
        //alloc more mem
        vec->allocCount += VECTOR_ALLOC_SIZE;
        char* tmpData = malloc(vec->allocCount * vec->size);
        memcpy(tmpData,vec->data,vec->allocCount - VECTOR_ALLOC_SIZE);
        free(vec->data);
        vec->data = tmpData;
    }
    //return &((char*)vec->data)[vec->last];
    return VecLast(vec);
}
void* VecLast(Vec* vec){
    //return &(((char*)vec->data)[vec->last]);
    return (((char*)vec->data) + vec->size * vec->last);
}

void VecDestroy(Vec* vec){
    free(vec->data);
    vec->size = 0;
    vec->last = 0;
    vec->allocCount = 0;
    vec->data = NULL;
}
