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
#define For_Rev_Each(list,iter) for(Iter iter=MakeReverseIter(&list);Dec(&iter);)
#define Iter_Val(iter,type) ((type*)(iter.this->data))

Link AddNode(List* list,int index,void* data,size_t dataSize);
Link AddIter(List* list,Iter* iter,void* data,size_t dataSize);//not done
Link PushBack(List* list,void* data,size_t dataSize);
Iter List_FindPointer(List* list,void* ptr);//searches the list for the data member to be equal to ptr
void RemoveElement(Iter* iter);//frees data
void RemoveElementNF(Iter* iter);//doesn't free data
void FreeList(List* list);

void* CreateBasket(size_t dataSize,void* data);//you are free to dealloate this, RemoveElement will also do that.

void NewIter(List* list,Iter* iter);
Iter MakeIter(List* list);
Iter MakeReverseIter(List* list);
int Dec(Iter* iter); //like Inc, but the other way, and returns 0 when it hits the beginning.
int Inc(Iter* iter);