#include "pool.h"
typedef struct{
    unsigned int itemSize,itemPoolCount;
    //unsigned int itemCount; Removing because sparse and packed sets have different 'itemCount's by
    //default.
    //List deleted;//This list is a pool of deleted entities ready to be created.
    //BAD IDEA ALERT! What if sparse had unsigned ints, but 0 means that it is invalid, and 1 is the 0th index.
    //there it is, gonna use it I guess.
    Pool sparse;//,versions;//These lists are pools that are indexed with entities.
                        //this means that when we need more entities, we will
                        //add another array to the list.
    Pool packed;//This is the packed list, it contains the actual component memory.
                //this scales with entities that *have* the component.
}PackedSet;//pretty much just or components

PackedSet CreatePackedSet(unsigned int itemSize,unsigned int itemPoolCount);
unsigned int PkdAddItem(PackedSet* set,void* component);