//
// Created by David on 9/13/2021.
//

#include "input.h"
#include "pool.h"

//Don't look at me like that. This isn't convoluted at all
char* GetLine(int* size){
    Pool buffer = CreatePool(1);
    while(1){
        char next = getc(stdin);
        if(next != '\n'){
            *((char*)PL_GetItem(buffer,PL_GetNextItem(&buffer))) = next;
            (*size)++;
        }else break;
    }
    char* buff = malloc(*size + 1);
    int left = *size;
    For_Each(buffer.list,iter){
        //goes through the arrays
        int start = *size - left;
        int end = *size >= left ? (left > POOL_SIZE ? POOL_SIZE : left) : *size;
        memcpy(buff + start, Iter_Val(iter,char),end);
        left -= *size >= left ? (left > POOL_SIZE ? POOL_SIZE : left) : *size;
    }
    buff[*size] = 0;
    FreeList(&buffer.list);
    return buff;
}
