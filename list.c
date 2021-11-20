#include <memory.h>
#include "list.h"

// 0 is before list, increments from there
Link AddNode(List* list,int index,void* data,size_t dataSize){
	//check if out of range
	if(index>list->count || index<0) {
        printf("index %d out of range!\n", index);
        return NULL;
    }
	Link newLink = malloc(sizeof(struct Link_t));
	newLink->data = data;
	newLink->dataSize = dataSize;
	newLink->root = list;
	if(list->start==NULL){
		newLink->last = NULL;
		newLink->next = NULL;
		list->start=newLink;
		list->end=newLink;
	}else if(index==list->count){//is pushback now
		newLink->last = list->end;
		newLink->next = NULL;
		list->end->next=newLink;
		list->end=newLink;
	}else//somewhere in the list
	{
		Link iter = list->start;
		//iterate to find target to move behind (0 will move to behind this list)
		for(int i = 0;i < index;i++){
			iter=iter->next;
		}
		//use iter to move behind
		if(iter==list->start)
			list->start=newLink;
		Link oldBehind=iter->last;
		newLink->last=oldBehind;
		newLink->next=iter;
		if(oldBehind)
			oldBehind->next=newLink;
		iter->last = newLink;
		
	}
	list->count++;
	return newLink;
}
Link PushBack(List* list,void* data,size_t dataSize){
	return AddNode(list,list->count,data,dataSize);
}

void NewIter(List* list,Iter* iter)
{
	iter->i=0;
	iter->root=list;
	iter->next=list->start;
	iter->last=NULL;
	iter->this=NULL;
}

Iter List_FindPointer(List* list,void* ptr){
    For_Each(*list,listIter){
        if(listIter.this->data == ptr){
            return listIter;
        }
    }
    printf("Failed to find ptr in list\n");
    return (Iter){0};
}

Iter MakeIter(List* list){
	Iter tmpIter={0};
    tmpIter.i = 0;//I don't like this. because it is unsigned. but it increments first anyways so whatever.
	tmpIter.root=list;
    tmpIter.this = list->start;
	if(tmpIter.this) tmpIter.next=tmpIter.this->next;
	else tmpIter.next = NULL;

	return tmpIter;
}
Iter MakeReverseIter(List* list){
    Iter tmpIter={0};
    tmpIter.i=(int)list->count-1;
    tmpIter.root = list;
    tmpIter.this = list->end;
    if(tmpIter.this) tmpIter.last = tmpIter.this->last;
    return tmpIter;
}
void _RemoveElement(Iter* iter,int doFreeData){
	if(iter->this==NULL){
		printf("failed to remove NULL (haven't iterated yet?)\n");
		return;
	}
	iter->root->count--;
	List* list = iter->root;
	Link link = iter->this;
	Link last = link->last;
	Link next = link->next;
	//patch up list
	if(last) last->next=next;
	if(next) next->last=last;
	if(link->last==NULL) list->start=next;
	if(link->next==NULL) list->end=last;
	if(doFreeData) free(link->data);
	free(link);
}
void RemoveElement(Iter* iter){
	_RemoveElement(iter,1);
}
void RemoveElementNF(Iter* iter){
	_RemoveElement(iter,0);
}
void* CreateBasket(size_t dataSize,void* data){
    void* basket = malloc(dataSize);
    memcpy(basket,data,dataSize);
    return basket;
}
void Dec(Iter* iter) { //like Inc, but the other way, and returns 0 when it hits the beginning.
    iter->next=iter->this;
    iter->this=iter->last;
    if(iter->last)iter->last=iter->last->last;
    iter->i--;
}
void Inc(Iter* iter){
    iter->last=iter->this;
	iter->this=iter->next;
	if(iter->next) iter->next=iter->next->next;
	//if(iter->last) iter->last=iter->last->next;
	iter->i++;
}
int ListCheck(Iter iter){
    return iter.this!=NULL;
}
//removes/frees everything in list
//warning, this will not deallocate ANY of the data's data (but will dealocate the member "data")
//if you want to deallocate any of that, you'll have to create the loop yourself
void FreeList(List* list){
	For_Each(*list,listIter){
		RemoveElement(&listIter);
	}
}
void* GetNth(List* list,unsigned int n){
    For_Each(*list,listIter){
        if(listIter.i == n){
            return listIter.this->data;
        }
    }
    return NULL;
}
Iter LinkToIter(Link link){
    Iter iter = {0};
    //maybe iterate to find 'i'
    iter.last = link->last;
    iter.next = link->next;
    iter.root = link->root;
    iter.this = link;
    return iter;
}
