#include "pool.h"

void *PL_GetItem(Pool pool,unsigned short eID) {//eID starts at 1
    //check to see if this is in currently allocated space
    //if it is not, throw an error
    if(eID == 0){
        printf("attempt to index pool with NULL entity (0)\n");
        return NULL;
    }
    if(eID > pool.list.count * POOL_SIZE){
        printf("Error getting unallocated eID\n");
        return NULL;
    }
    //eID / POOL_SIZE -> how many pools can you fit in it?
    //Ceilf(^) -> ok but we want an integer. if eID is 100, ^ is 1, so Ceilf is 1; any more and it is 2
    //-1 -> ok but we want zero to be the ifrst pool for multiplication/offset reasons
    int poolIndex = (int)ceilf(((float)eID)/POOL_SIZE)-1;
    //poolIndex*POOL_SIZE -> Get pool offset in id's
    //eID - ^ -> remove the previous pools-worth of id's from eID. get only the index into the pool eID is in.
    //-1 -> if poolIndex = 0 and eID is 1, it should result in 0. because it is index space.
    int lastItemIndex = eID - (poolIndex*POOL_SIZE)-1;
    char* byteData = NULL;
    For_Each(pool.list,iter){
        if(iter.i == poolIndex){
            //byteData = &(Iter_Val(iter,char)[lastItemIndex]);
            byteData = &(((char*)iter.this->data)[lastItemIndex*pool.itemSize]);
            break;
        }
    }
    if(byteData == NULL) {
        printf("Oh no! Couldn't find the pool array that holds the last item!\n");
    }
    return byteData;
}

void *PL_GetLastItem(Pool pool) {
    return PL_GetItem(pool,pool.itemCount+1);
}

void _PL_NewArray(Pool* pool) {
    //this will always create a new array in the pool
    if(pool->itemCount >= pool->list.count*POOL_SIZE) {
        PushBack(&pool->list, malloc(pool->itemSize * POOL_SIZE), pool->itemSize);
        memset(pool->list.end->data,0,POOL_SIZE*pool->itemSize);//initialize to zero
    }else printf("Did not need to allocate new memory. why?\n");
}

unsigned short PL_GetNextItem(Pool* pool) {
    //check to see if pool can hold another item
    if(pool->itemCount == pool->list.count*POOL_SIZE){
        //allocate memory
        _PL_NewArray(pool);
    }else if(pool->itemCount > pool->list.count*POOL_SIZE){
        //this should NEVER happen. Just printing cause this might bite me in the ass later
        printf("Oh no! Pool didn't resize when adding an item. things are bad. check %d %s\n",__LINE__,__FILE__);
    }
    return ++pool->itemCount;
}

void *PL_GetFirstItem(Pool pool) {
    return pool.list.start->data;
}

Pool CreatePool(unsigned int itemSize) {
    Pool result = {0};
    result.itemSize=itemSize;
    //PushBack(&result.list,malloc(result.itemSize*POOL_SIZE),result.itemSize);
    //memset(result.list.start->data,0,POOL_SIZE*result.itemSize);
    return result;
}
