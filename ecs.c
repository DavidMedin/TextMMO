#include "ecs.h"
#include <stdarg.h>


List components ={0};
void ECSStartup(){
    versions = CreatePool(sizeof(short));
    deleted = CreatePool(sizeof(short));
}
int RegisterComponent(int typesize,componentInitFunc initFunc,componentDestroyFunc destroyFunc){
    static int componentID = 0;
    Component* comp = malloc(sizeof(Component));
    comp->initFunc = initFunc;
    comp->destroyFunc = destroyFunc;//not required to be specified
    PackedSet* set = &comp->data;
    set->itemSize = typesize+sizeof(int);
    set->itemPoolCount = POOL_SIZE;
    set->sparse = CreatePool(sizeof(short));
    set->packed = CreatePool(set->itemSize);//sudo struct for the win
    PushBack(&components,comp,sizeof(Component));
    return componentID++;
}
int IsEntityValid(int entity){
    if((unsigned short)entity == 0){
        return 0;
    }
    void* registeredEntity = PL_GetItem(versions,(short)entity);
    if(registeredEntity == 0){
        return 0;
    }
    short trueVersion = *((short*)registeredEntity);
    short testVersion = ((short*)&entity)[1];
    if(trueVersion != testVersion){
        return 0;
    }else{
        return 1;
    }
}

int CreateEntity(){
    //Is there anything in the deleted array?
    unsigned short eID;
    if(deleted.itemCount != 0){
        //yup!
        //Use the first as our new entity, and move the last in the array to the beginning
        eID = *(short*)PL_GetFirstItem(deleted);
        *(short*)PL_GetFirstItem(deleted) = *(short*)PL_GetLastItem(deleted);
        deleted.itemCount--;
        //potentially free memory in pool
    }else{
        //Nope
        //resize Version and components.sparse as needed. or anything else that is scaled with entity count.
        eID = PL_GetNextItem(&versions);
        For_Each(components,iter){
            Component* comp = Iter_Val(iter,Component);
            PL_GetNextItem(&comp->data.sparse);//This increments the pool item count. Don't know the reprecussions
            // for that.
        }
    }
    int entity = (int)eID;
    short* entityVersion = (short*)PL_GetItem(versions,eID);
    ((short*)&entity)[1] = *entityVersion;
    return entity;
}
void DestroyEntity(int entityID){
    //check all component sparse sets and find the ones it is in. exterminate
    if(!IsEntityValid(entityID)){
        printf("Tried to delete invalid entity! E:%d V:%d\n",ID(entityID), VERSION(entityID));
        return;
    }
    For_Each(components,iter){
        Component* comp = Iter_Val(iter,Component);
        unsigned short* sparseSlot = (unsigned short*)PL_GetItem(comp->data.sparse,(unsigned short)entityID);
        if(*sparseSlot != 0){
            //reorganize sparse and packed sets
            void* packedSlot = PL_GetItem(comp->data.packed,(unsigned short)(*sparseSlot-1));
            //get last packed slot
            unsigned int componentCount = comp->data.packed.itemCount;
            void* lastSlot = PL_GetItem(comp->data.packed,(unsigned short)(componentCount));
            int lastEntity = *(int*)lastSlot;
            void* lastSparsePointer = PL_GetItem(comp->data.sparse,(unsigned short)lastEntity);
            // space to index space

            //deconstruct
            if(comp->destroyFunc != NULL){
                comp->destroyFunc(((char*)packedSlot)+sizeof(short));//packed(+short) is where the data lives
            }

            memcpy(packedSlot,lastSlot,comp->data.packed.itemSize);
            *(unsigned short*)lastSparsePointer = *sparseSlot;
            *sparseSlot = 0;//this is what we save 0 for
            comp->data.packed.itemCount--;
            //had to cast to int because unsigned ints were overflowing. int is good math.
           while((short)comp->data.packed.itemCount <= ((short)comp->data.packed.list.count-1)*POOL_SIZE){
                Pool* set = &comp->data.packed;
                Iter removeIter = {0};
                removeIter.this = set->list.end;
                removeIter.next = set->list.end->next;
                removeIter.last = set->list.end->last;
                removeIter.root = &set->list;
                removeIter.i = -1;
                RemoveElement(&removeIter);
            }

            unsigned short* nextDeleted = PL_GetItem(deleted,PL_GetNextItem(&deleted));
            *nextDeleted = (unsigned short)entityID;
        }
    }
    //find entity in versions
    short* version = ((short*) PL_GetItem(versions,(short)entityID));
    (*version)++;
}
void AddComponent(int entityID,int componentID){
    if(!IsEntityValid(entityID)){
        printf("Tried to add component to invalid entity! E:%d V:%d\n",ID(entityID),VERSION(entityID));
        return;
    }
    For_Each(components,iter){
        if(componentID == iter.i){
            //this component
            Component* comp = Iter_Val(iter,Component);
            unsigned short compID = PL_GetNextItem(&comp->data.packed);//the index (+1) into the packed set where we
            // will write our shit
            unsigned short* sparseEntry = PL_GetItem(comp->data.sparse,(short)entityID);
            if(sparseEntry==NULL){
                printf("Error trying to get entity (%d) from unallocated space\n",(unsigned short)entityID);
            }
            *sparseEntry = (unsigned short)(compID+1);//+1 because 0 is reserved for INVALID. everything is
            // +1 for indexing set.packed.
            void* packedEntry = PL_GetItem(comp->data.packed,(unsigned short)(compID));
            *(int*)packedEntry = entityID;//int for entire entity id, eID and version
            comp->initFunc(((char*)packedEntry)+sizeof(int));
        }
    }
}
void _CallSystem(SystemFunc func,int componentID,...){
    For_Each(components,iter){
        if(iter.i==componentID){
            //this is the component in question
            Component* compType = Iter_Val(iter,Component);
            int itemsLeft = compType->data.packed.itemCount;//we're using an int because it can hold all of unsigned
            // short, and can have negatives
            For_Each(compType->data.packed.list,arrayIter){
                char* array = Iter_Val(arrayIter,char);
                for(unsigned short i = 0;i < (unsigned short)((itemsLeft < POOL_SIZE) ? itemsLeft : POOL_SIZE);i++){
                    //i is the entity ID, so check that it has all components that we want
                    va_list vl;
                    va_start(vl,componentID);
                    int component;
                    int noHave = 0;
                    int entity =  *(int*)&array[i*compType->data.itemSize];
                    while((component = va_arg(vl,Entity)) != -1){
                        if(HasComponent(entity,component) == 0) {
                            //this entity doesn't have this component
                            noHave = 1;
                            break;
                        }
                    }
                    if(!noHave){
                        func(entity);
                    }
                }
                itemsLeft -=  ((itemsLeft < POOL_SIZE) ? itemsLeft : POOL_SIZE);
                if(itemsLeft <= 0) break;//ideally, we would delete extra arrays, but whatever.
            }
            return;//catches both done with for loop and count == zero
        }
    }
    printf("Couldn't find component with ID %d\n",componentID);
}


void* GetComponent(int componentID,int entityID){
    if(!IsEntityValid(entityID)){
        //this is *a* way of testing if an entity is invalid
        //printf("Tried to get component of invalid entity! E:%d V:%d\n",ID(entityID),VERSION(entityID));
        return NULL;
    }
    For_Each(components,iter){
        if(iter.i == componentID){
            Component* comp = Iter_Val(iter,Component);
            unsigned short sparseIndex = *(unsigned short*)PL_GetItem(comp->data.sparse,(unsigned short)entityID);
            if(sparseIndex != 0){
                void* componentData = PL_GetItem(comp->data.packed,(unsigned short)(sparseIndex-1));//-1 because 0 is
                // NULL.
                return (char*)componentData+sizeof(int);
            }else{
                //this entity doesn't have this component
                printf("This entity (%d) doesn't have this component! (%d)\n",entityID,componentID);
                return NULL;
            }
        }
    }
    printf("No component with id %d\n",componentID);
    return NULL;
}

SysIter ForSysCreateIter(int compID){
    SysIter newIter = {0};
    //find Component
    For_Each(components,compIter){
        if(compIter.i == compID){
            //this is our component
            newIter.arrayIter = MakeIter(&Iter_Val(compIter,Component)->data.packed.list);
            Inc(&newIter.arrayIter);//big dumb
            newIter.ptr = (int*)newIter.arrayIter.this->data+1;
            newIter.ent = *((int*)newIter.ptr-1);
            newIter.comp = Iter_Val(compIter,Component);
        }
    }
    return newIter;
}
int ForSysTest(SysIter iter,int componentID){
    //test if we have reached the end
    if(iter.i == iter.comp->data.packed.itemCount){//i is an index, not a count. +1
        //we have reached the end
        return 0;
    }
    return 1;
}
void ForSysInc(SysIter* iter,int componentID){
    //use i to find if we need to change array
    if(((iter->i+1) % POOL_SIZE) == 0){
        //might not work, but this should mean that it is time to change arrays
        if(Inc(&iter->arrayIter) == 0){
            //no more arrays, do something that will cause ForSysTest to trip
            //return;
        }
        //update ptr
        iter->ptr = ((int*)iter->arrayIter.this->data+1);
    }else
        iter->ptr = iter->ptr+iter->comp->data.itemSize;//inc ptr
    iter->i++;
    iter->ent = *((int*)iter->ptr-1);//inc net
}

int HasComponent(Entity ent, int compID) {
    For_Each(components,compIter){
        if(compIter.i == compID){
            //this is our component
            short* sparseSlot = PL_GetItem(Iter_Val(compIter,Component)->data.sparse,ent);
            if(*sparseSlot == 0){
               return 0;//this entity doesn't have this component
            }else return 1;
        }
    }
    printf("Tried to call HasComponent with a component (%d) that doesn't exist!\n",compID);
    return 0;
}
