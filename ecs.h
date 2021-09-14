#include "packedSet.h"
#define ID(eID) (short)eID
#define VERSION(eID) ((short*)&eID)[1]
typedef void (*componentInitFunc)(void*);
typedef void(*SystemFunc)(int,void*);//int is the entity ID.
typedef int Entity;
typedef struct{
    PackedSet data;
    componentInitFunc initFunc;
}Component;

extern List components;
Pool deleted;
Pool versions;

#define For_Comp()

void ECSStartup();
int RegisterComponent(int typesize,componentInitFunc initFunc);
int IsEntityValid(int entity);
int CreateEntity();
void DestroyEntity(int entityID);
void AddComponent(int entityID,int componentID);
void CallSystem(SystemFunc func,int componentID,void* randomData);
void* GetComponent(int componentID,int entityID);
