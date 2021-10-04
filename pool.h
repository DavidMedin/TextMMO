#pragma once
#include "list.h"
#include <math.h>
#include <memory.h>
#define POOL_SIZE 5
//A Pool is a list of arrays which all have the same length. They all hold one data type.
//The idea is it is like a list that is perhaps more efficient in speed, but not memory size.
//Maybe. Speculative. Whatever I am doing it; don't @ me.
typedef struct{List list;unsigned int itemSize;unsigned short itemCount;}Pool;
Pool CreatePool(unsigned int itemSize);
void* PL_GetFirstItem(Pool pool);
void* PL_GetItem(Pool pool,unsigned short eID);
void* PL_GetLastItem(Pool pool);
void _PL_NewArray(Pool* pool);
//gets the next item after the last one. Will automatically allocate new memory if previous arrays
//of memory are full.
unsigned short PL_GetNextItem(Pool* pool);
