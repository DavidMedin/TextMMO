#include "list.h"
typedef struct{
    unsigned int itemSize,itemPoolCount;
    unsigned int itemCount;
    //List deleted;//This list is a pool of deleted entities ready to be created.
    List sparse;//,versions;//These lists are pools that are indexed with entities.
                        //this means that when we need more entities, we will
                        //add another array to the list.
    List packed;//This is the packed list, it contains the actual component memory.
                //this scales with entities that *have* the component.
}PackedSet;//pretty much just or components

PackedSet CreatePackedSet(unsigned int itemSize,unsigned int itemPoolCount);
unsigned int PkdAddItem(PackedSet* set,void* component);