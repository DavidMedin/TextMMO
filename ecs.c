#include "ecs.h"
#include <stdarg.h>
#include "log.h"


List components ={0};
void ECSStartup(){
    //TODO: Combine these (See https://skypjack.github.io/2019-05-06-ecs-baf-part-3/)
    versions = CreatePool(sizeof(short));
    deleted = CreatePool(sizeof(short));
}
CompID RegisterComponent(int typesize,componentInitFunc initFunc,componentDestroyFunc destroyFunc){
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
    log_info("registering component of size %d",typesize);
    return componentID++;
}
int IsEntityValid(Entity entity){
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
    log_debug("Deleted list is {%d} long",deleted.itemCount);
    if(deleted.itemCount != 0){
        //yup!
        //Use the first as our new entity, and move the last in the array to the beginning
        eID = *(short*)PL_GetFirstItem(deleted);
        //*(short*)PL_GetFirstItem(deleted) = *(short*)PL_GetLastItem(deleted);
        short* first = PL_GetFirstItem(deleted);
        short* last = PL_GetLastItem(deleted);
        *first = *last;
        deleted.itemCount--;
        //potentially free memory in pool
    }else{
        //Nope
        //resize Version and components.sparse as needed. or anything else that is scaled with entity count.
        log_debug("Version is of size {%d}, using or entity ID",versions.itemCount);
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
    log_info("Creating entity {E: %d - V: %d}",ID(entity),VERSION(entity));
    return entity;
}
void _RemoveComponent(Entity entity,Iter iterator){
    Component* comp = Iter_Val(iterator,Component);
    unsigned short *sparseSlot = (unsigned short *) PL_GetItem(comp->data.sparse, (unsigned short) entity);
    if (*sparseSlot != 0) {
        //reorganize sparse and packed sets
        void *packedSlot = PL_GetItem(comp->data.packed, (unsigned short) (*sparseSlot - 1));
        //get last packed slot
        unsigned int componentCount = comp->data.packed.itemCount;
        void *lastSlot = PL_GetItem(comp->data.packed, (unsigned short) (componentCount));
        int lastEntity = *(int *) lastSlot;
        void *lastSparsePointer = PL_GetItem(comp->data.sparse, (unsigned short) lastEntity);
        // space to index space

        //deconstruct
        if (comp->destroyFunc != NULL) {
            comp->destroyFunc(((char *) packedSlot) + sizeof(int));//packed(+short) is where the data lives
        }
        //writing
        memcpy(packedSlot, lastSlot, comp->data.packed.itemSize);
        *(unsigned short *) lastSparsePointer = *sparseSlot;
        *sparseSlot = 0;//this is what we save 0 for
        comp->data.packed.itemCount--;
        //had to cast to int because unsigned ints were overflowing. int is good math.
        while ((short) comp->data.packed.itemCount <= ((short) comp->data.packed.list.count - 1) * POOL_SIZE) {
            Pool *set = &comp->data.packed;
            Iter removeIter = {0};
            removeIter.this = set->list.end;
            removeIter.next = set->list.end->next;
            removeIter.last = set->list.end->last;
            removeIter.root = &set->list;
            removeIter.i = -1;
            RemoveElement(&removeIter);
            log_info("Deallocated array from packed list from component {%d}\n\tDown"
                     " to size of %d.",iterator.i,set->itemCount);
        }

    }else{
        log_error("Attempted to delete a component {%d} that didn't exist!",iterator.i);
        //printf("Attempted to delete a component that didn't exist {%d}\n",iterator.i);
    }
    log_info("Removed compenent {%d} from entity {E: %d - V: %d}\n\tComponent now has {%d} entities with it.",iterator
    .i,ID(entity),VERSION(entity),comp->data.packed.itemCount);
}
void DestroyEntity(Entity entity){//actually fully destroys an enitity
    //check all component sparse sets and find the ones it is in. exterminate
    if(!IsEntityValid(entity)){
        //printf("Tried to delete invalid entity! E:%d V:%d\n",ID(entity), VERSION(entity));
        log_error("Attempted to delete an invalid entity {E: %d - V: %d}!",ID(entity), VERSION(entity));
        return;
    }
    For_Each(components,iter){
        if(HasComponent(entity,iter.i)){
            _RemoveComponent(entity,iter);
        }
    }
    unsigned short *nextDeleted = PL_GetItem(deleted, PL_GetNextItem(&deleted));
    *nextDeleted = (unsigned short) entity;
    //find entity in versions
    short* version = ((short*) PL_GetItem(versions,(short)entity));
    (*version)++;
    log_debug("Incremented version for entity {E: %d} to {V: %d}",ID(entity),*version);
    log_info("Successfully destroyed entity {E: %d - V: %d}",ID(entity),VERSION(entity));
}
void RemoveComponent(Entity entity,CompID component){
    For_Each(components,compIter){
        if(compIter.i == component){
            _RemoveComponent(entity,compIter);
            log_info("Removed component {%d} from entity {E: %d - V: %d}",component,ID(entity),VERSION(entity));
            return;
        }
    }
    log_error("Attempted to remove a component {%d} from entity {E: %d - V: %d} where the component that didn't "
              "exist!",ID(entity),VERSION(entity));
}
void* AddComponent(Entity entity,CompID component){
    if(!IsEntityValid(entity)){
        log_error("Tried to add component {%d} to invalid entity {E: %d - V: %d}!\n",component,ID(entity),VERSION
        (entity));
        return NULL;
    }
    For_Each(components,iter){
        if(component == iter.i){
            //this component
            Component* comp = Iter_Val(iter,Component);
            unsigned short compID = PL_GetNextItem(&comp->data.packed);//the index (+1) into the packed set where we
            // will write our shit
            unsigned short* sparseEntry = PL_GetItem(comp->data.sparse,(short)entity);
            if(sparseEntry==NULL){
                log_error("Error trying to get entity {E: %d - V: %d} from unallocated space\n",ID(entity), VERSION
                (entity));
            }
            *sparseEntry = (unsigned short)(compID+1);//+1 because 0 is reserved for INVALID. everything is
            // +1 for indexing set.packed.
            void* packedEntry = PL_GetItem(comp->data.packed,(unsigned short)(compID));
            *(int*)packedEntry = entity;//int for entire entity id, eID and version
            comp->initFunc(((char*)packedEntry)+sizeof(int));
            log_info("Added component {%d} to entity {E: %d - V: %d}\n\tComponent now has {%d} entities with it.",iter
            .i,ID(entity),VERSION(entity),comp->data.packed.itemCount);
            return ((char*)packedEntry)+sizeof(int);
        }
    }
}
void _CallSystem(SystemFunc func, CompID component, ...){
    //For_Each(components,iter){
    //    if(iter.i == component){
    //        //this is the component in question
    //        Component* compType = Iter_Val(iter,Component);
    //        int itemsLeft = compType->data.packed.itemCount;//we're using an int because it can hold all of unsigned
    //        // short, and can have negatives
    //        For_Each(compType->data.packed.list,arrayIter){
    //            char* array = Iter_Val(arrayIter,char);
    //            for(unsigned short i = 0;i < (unsigned short)((itemsLeft < POOL_SIZE) ? itemsLeft : POOL_SIZE);i++){
                    //i is the entity ID, so check that it has all components that we want
    For_System(component,compIter){

                    va_list vl;
                    va_start(vl, component);
                    CompID thisComponent;
                    //int entity =  *(int*)&array[i*compType->data.itemSize];
                    int entity = compIter.ent;
                    while((thisComponent = va_arg(vl, Entity)) != -1){
                        if(HasComponent(entity, thisComponent) == 0) {
                            //this entity doesn't have this component
                            goto dontHave;
                        }
                    }
                    func(entity);
                    dontHave:;
    //           }
    //            itemsLeft -=  ((itemsLeft < POOL_SIZE) ? itemsLeft : POOL_SIZE);
    //            if(itemsLeft <= 0) break;//ideally, we would delete extra arrays, but whatever.
    //        }
    //        return;//catches both done with for loop and count == zero
    //    }
    }
    //printf("Couldn't find component with ID %d\n", component);
}


void* GetComponent(Entity entity,CompID component){
    if(!IsEntityValid(entity)){
        //this is *a* way of testing if an entity is invalid
        return NULL;
    }
    For_Each(components,iter){
        if(iter.i == component){
            Component* comp = Iter_Val(iter,Component);
            unsigned short sparseIndex = *(unsigned short*)PL_GetItem(comp->data.sparse,(unsigned short)entity);
            if(sparseIndex != 0){
                void* componentData = PL_GetItem(comp->data.packed,(unsigned short)(sparseIndex-1));//-1 because 0 is
                // NULL.
                return (char*)componentData+sizeof(int);
            }else{
                //this entity doesn't have this component
                log_error("Failed to get component {%d} from entity {E: %d - V: %d}!",component,ID(entity),VERSION
                (entity));
                return NULL;
            }
        }
    }
    log_error("Tried to get component {%d} that doesn't exist!",component);
    return NULL;
}

SysIter ForSysCreateIter(CompID component){
    SysIter newIter = {0};
    //find Component
    For_Each(components,compIter){
        if(compIter.i == component){
            //this is our component
            Component* cmp = Iter_Val(compIter,Component);
            newIter.i=cmp->data.packed.itemCount-1;
            List* list = &cmp->data.packed.list;
            newIter.arrayIter = MakeReverseIter(list);
            Dec(&newIter.arrayIter);//big dumb
            newIter.comp = cmp;
            if(list->count == 0){
                return newIter;
            }
            unsigned int lastCount = cmp->data.packed.itemCount - ((list->count-1) *cmp->data.itemPoolCount);//the count
            // of elements of the last array
            newIter.ptr = ((char*)newIter.arrayIter.this->data + sizeof(int) + (lastCount-1)*cmp->data.itemSize);
                //NIGHTMARE NIGHTMARE NIGHTMARE NIGHTMARE NIGHTMARE NIGHTMARE NIGHTMARE NIGHTMARE NIGHTMARE
            newIter.ent = *((int*)newIter.ptr-1);
            break;
        }
    }
    return newIter;
}
int ForSysTest(SysIter iter){
    //test if we have reached the end
    if(iter.i == -1){
        return 0;
    }
    return 1;
}
void ForSysDec(SysIter* iter){
    //use i to find if we need to change array
    if(iter->i == 0) {iter->i--; return;}//we are done, just dec, and don't touch anything to do with pointers.
    if(((iter->i) % POOL_SIZE) == 0){
        //might not work, but this should mean that it is time to change arrays
        if(Dec(&iter->arrayIter) == 0){
            //no more arrays, do something that will cause ForSysTest to trip
            //return;
            //printf("done with array\n");
        }
        //update ptr
        iter->ptr = ((char*)iter->arrayIter.this->data + sizeof(int) + (iter->comp->data.itemPoolCount-1)*
                iter->comp->data.itemSize);
        //iter->ptr = ((int*)iter->arrayIter.this->data+1);//+1 moves ptr to the data.+0 would point to an Entity ID.
    }else
        iter->ptr = iter->ptr-iter->comp->data.itemSize;//dec ptr
    iter->i--;
    iter->ent = *((int*)iter->ptr-1);//inc net
}

int HasComponent(Entity entity, CompID component) {
    For_Each(components,compIter){
        if(compIter.i == component){
            //this is our component
            short* sparseSlot = PL_GetItem(Iter_Val(compIter,Component)->data.sparse, entity);
            if(*sparseSlot == 0){
               return 0;//this entity doesn't have this component
            }else return 1;
        }
    }
    log_error("Tried to call HasComponent with a component {%d} that doesn't exist!\n", component);
    return 0;
}

