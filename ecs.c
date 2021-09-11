#include "ecs.h"


List components ={0};
void ECSStartup(){
    versions = CreatePool(sizeof(short));
    deleted = CreatePool(sizeof(short));
}
int RegisterComponent(int typesize,componentInitFunc initFunc){
    static int componentID = 0;
    Component* comp = malloc(sizeof(Component));
    comp->initFunc = initFunc;
    PackedSet* set = &comp->data;
    set->itemSize = typesize+sizeof(int);
    set->itemPoolCount = POOL_SIZE;
    set->sparse = CreatePool(sizeof(short));
    set->packed = CreatePool(set->itemSize);//sudo struct for the win
    PushBack(&components,comp,sizeof(Component));
    return componentID++;
}
int IsEntityValid(int entity){
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
    short eID;
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
        short* sparseSlot = (short*)PL_GetItem(comp->data.sparse,(short)entityID);
        if(*sparseSlot != 0){
            //reorganize sparse and packed sets
            void* packedSlot = PL_GetItem(comp->data.packed,(short)(*sparseSlot-1));
            //get last packed slot
            unsigned int componentCount = comp->data.packed.itemCount;
            void* lastSlot = PL_GetItem(comp->data.packed,(short)(componentCount-1));
            int lastEntity = *(int*)lastSlot;
            void* lastSparsePointer = PL_GetItem(comp->data.sparse,(short)lastEntity);
            memcpy(packedSlot,lastSlot,comp->data.packed.itemSize);
            *(short*)lastSparsePointer = *sparseSlot;
            *sparseSlot = 0;//this is what we save 0 for
            comp->data.packed.itemCount--;
            //TODO: potentially free data.
           // while(comp->data.packed.itemCount <= (comp->data.packed.list.count-1)*POOL_SIZE){
           //     Pool* set = &comp->data.packed;
           //     Iter removeIter = {0};
           //     removeIter.this = set->list.end;
           //     removeIter.next = set->list.end->next;
           //     removeIter.last = set->list.end->last;
           //     removeIter.root = &set->list;
           //     removeIter.i = -1;
           //     RemoveElement(&removeIter);
           // }

            short* nextDeleted = PL_GetItem(deleted,PL_GetNextItem(&deleted));
            *nextDeleted = (short)entityID;
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
            short compID = PL_GetNextItem(&comp->data.packed);
            short* sparseEntry = PL_GetItem(comp->data.sparse,(short)entityID);
            if(sparseEntry==NULL){
                printf("Error trying to get entity (%d) from unallocated space\n",(short)entityID);
            }
            *sparseEntry = (short)(compID+1);//+1 because 0 is reserved for INVALID. everything is
            // +1 for indexing set.packed.
            void* packedEntry = PL_GetItem(comp->data.packed,(short)(compID));
            *(int*)packedEntry = entityID;
            comp->initFunc(((char*)packedEntry)+sizeof(int));
        }
    }
}
void CallSystem(SystemFunc func,int componentID){
    For_Each(components,iter){
        if(iter.i==componentID){
            //this is the component in question
            Component* compType = Iter_Val(iter,Component);
            unsigned int itemsLeft = compType->data.packed.itemCount;
            For_Each(compType->data.packed.list,arrayIter){
                char* array = Iter_Val(arrayIter,char);
                for(int i = 0;i < ((itemsLeft < POOL_SIZE) ? itemsLeft : POOL_SIZE);i++){
                    func(*(int*)&array[i*compType->data.itemSize]);
                }
                itemsLeft -=  ((itemsLeft < POOL_SIZE) ? itemsLeft : POOL_SIZE);
                if(itemsLeft <= 0) return;//ideally, we would delete extra arrays, but whatever.
            }
        }
    }
    printf("Couldn't find component with ID %d\n",componentID);
}
void* GetComponent(int componentID,int entityID){
    if(!IsEntityValid(entityID)){
        printf("Tried to get component of invalid entity! E:%d V:%d\n",ID(entityID),VERSION(entityID));
        return NULL;
    }
    For_Each(components,iter){
        if(iter.i == componentID){
            Component* comp = Iter_Val(iter,Component);
            short sparseIndex = *(short*)PL_GetItem(comp->data.sparse,(short)entityID);
            if(sparseIndex != 0){
                void* componentData = PL_GetItem(comp->data.packed,(short)(sparseIndex-1));//-1 because 0 is NULL.
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
