#pragma once
#include <stdio.h>
#include <stdlib.h>
// #include "luaManager.h"
typedef struct Link_t* Link;
typedef struct{
	Link start;
	Link end;
	unsigned int count;
}List;
typedef struct Link_t{
	List* root;
	struct Link_t* next;
	struct Link_t* last;
	void* data;
	size_t dataSize; 
}*Link;
typedef struct{
	Link next;
	Link last;
	Link this;
	List* root;
	int i;
}Iter;

#define For_Each(list,iter) for(Iter iter=MakeIter(&list);Inc(&iter);)
#define Iter_Val(iter,type) ((type*)(iter.this->data))

Link AddNode(List* list,int index,void* data,size_t dataSize);
Link AddIter(List* list,Iter* iter,void* data,size_t dataSize);//not done
Link PushBack(List* list,void* data,size_t dataSize);
void RemoveElement(Iter* iter);
void RemoveElementNF(Iter* iter);
void FreeList(List* list);

void NewIter(List* list,Iter* iter);
Iter MakeIter(List* list);
int Inc(Iter* iter);