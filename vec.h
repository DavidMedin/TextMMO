#pragma once
#define VECTOR_ALLOC_SIZE 100
typedef struct{
    void* data;
    unsigned int last;
    unsigned int size;//size of item
    unsigned int allocCount;
}Vec;

#define VecGet(vector,index) ((char*)vector.data+index*vector.size)
Vec VecMake(int size,int initCount);//size is the size of the elements, and initcount is how much to allocate now.
void* VecNext(Vec* vec);
void* VecLast(Vec* vec);
void VecDestroy(Vec* vec);