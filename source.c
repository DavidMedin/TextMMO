#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "packedSet.h"
/*
	TODO:
		-dynamic type allocation
		-dynamic entity pool allocation
		-packed list of components
		-entity gap filling (versioning?)
*/
#define MAX_ENTITIES 64
#define MAX_COMPONENTS 8
typedef void (*componentInitFunc)(void*);
typedef struct{
    PackedSet data;
	componentInitFunc initFunc;
}Component;
List components = {0};//the components. ta da
Pool deleted = {0};
Pool versions = {0};//holds the current valid versions for all entities.

//v doesn't exist anymore
char entities[MAX_COMPONENTS][MAX_ENTITIES];//describes what components belong to the entity

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
	memset(entities[entityID],0,MAX_COMPONENTS);
	printf("destroyed entity!\n");
}
void AddComponent(int entityID,int componentID){
    For_Each(components,iter){
        if(componentID == iter.i){
            //this component
            Component* comp = Iter_Val(iter,Component);
            short compID = PL_GetNextItem(&comp->data.packed);
            unsigned int* sparseEntry = PL_GetItem(comp->data.sparse,(short)entityID);
            if(sparseEntry==NULL){
                printf("Error trying to get entity (%d) from unallocated space\n",(short)entityID);
            }
            *sparseEntry = compID+1;//+1 because 0 is reserved for INVALID. everything is
            // +1 for indexing set.packed.
            void* packedEntry = PL_GetItem(comp->data.packed,(short)(compID));
            *(int*)packedEntry = entityID;
            comp->initFunc(((char*)packedEntry)+sizeof(int));
        }
    }
}
typedef void(*SystemFunc)(int);//int is the entity ID.
void CallSystem(SystemFunc func,int componentID){
    For_Each(components,iter){
        if(iter.i==componentID){
            //this is the component in question
            Component* compType = Iter_Val(iter,Component);
            unsigned int itemsLeft = compType->data.packed.itemCount;
            For_Each(compType->data.packed.list,arrayIter){
                char* array = Iter_Val(arrayIter,char);
                for(int i = 0;i < ((itemsLeft < POOL_SIZE) ? itemsLeft : POOL_SIZE);i++){
                    func((int)(array[i*compType->data.itemSize]));
                }
                itemsLeft -=  ((itemsLeft < POOL_SIZE) ? itemsLeft : POOL_SIZE);
                if(itemsLeft <= 0) return;//ideally, we would delete extra arrays, but whatever.
            }
        }
    }
    printf("Couldn't find component with ID %d\n",componentID);
}
void* GetComponent(int componentID,int entityID){
    For_Each(components,iter){
        if(iter.i == componentID){
            Component* comp = Iter_Val(iter,Component);
            short sparseIndex = *(short*)PL_GetItem(comp->data.sparse,(short)entityID);
            void* componentData = PL_GetItem(comp->data.packed,(short)(sparseIndex-1));//-1 because 0 is NULL.
            return (char*)componentData+sizeof(int);
        }
    }
    printf("No component with id %d\n",componentID);
    return NULL;
}

//------------------
int humanID;
typedef struct{
	int health;
	float height;
    char* name;
}Human;


void HumanInit(void* component){
	Human* hum= component;
	hum->health = 100;
	hum->height = 5.0f + 9.0f/12.0f;
    hum->name = "Joe";
	printf("Initialized the Human!\n");
}
void HumanUpdate(int entityID){
	Human* human = GetComponent(humanID,entityID);
	printf("%s is %.2f meters tall\n",human->name,human->height);
}

int talkID;
typedef struct{
    char interactKey;
}Talkable;

void TalkableInit(void* component){
    Talkable* talk = component;
    talk->interactKey = 'e';
}
void InteractSystem(int eID){
    Talkable* talk = GetComponent(talkID,eID);
    printf("did you hit the %c key?\n",talk->interactKey);
}

void ECSStartup(){
    versions = CreatePool(sizeof(short));
}

int main(int argc,char** argv){
    setbuf(stdout,0);//bruh why do I have to do this?
    ECSStartup();

    humanID = RegisterComponent(sizeof(Human),HumanInit);
    printf("The human ID is %d\n",humanID);
    talkID = RegisterComponent(sizeof(Talkable),TalkableInit);
    printf("The talkable ID is %d\n",talkID);

    int jim = CreateEntity();
    printf("Jim's entity ID is %d, and his version is %d\n",(short)jim,((short*)(&jim))[1]);
    int dave = CreateEntity();
    printf("Dave's entity ID is %d, and his version is %d\n",(short)dave,((short*)(&dave))[1]);

    AddComponent(jim,humanID);
    AddComponent(dave,humanID);
    AddComponent(dave,talkID);

    Human* jimHuman = (Human*) GetComponent(humanID,jim);
    jimHuman->name = "Jim";
    Human* daveHuman = (Human*) GetComponent(humanID,dave);
    daveHuman->name = "Dave";


    CallSystem(HumanUpdate,humanID);
    CallSystem(InteractSystem,talkID);
    return 0;
}