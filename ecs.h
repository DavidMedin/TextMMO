#pragma once
#include "packedSet.h"
#define ID(eID) (short)eID
#define VERSION(eID) ((short*)&eID)[1]

#define ENTFROMCOMP(comp) (int*)(((char*)comp)-sizeof(int))

typedef void (*componentInitFunc)(void*);
typedef void (*componentDestroyFunc)(void*);
typedef void(*SystemFunc)(int);//int is the entity ID.
typedef int Entity;
typedef int CompID;
typedef struct{
    PackedSet data;
    componentInitFunc initFunc;
    componentDestroyFunc destroyFunc;
}Component;

extern List components;
Pool deleted;
Pool versions;

#define For_System(component,iter) for(SysIter iter = ForSysCreateIter(component);ForSysTest(iter); \
ForSysDec(&iter))
#define SysIterVal(sysIter,type) ((type*)sysIter.ptr)
#define For_List_System(component,list,iter) for(SysIter iter = ForListSysCreateIter(component,&list);ForListSysTest(iter);ForListSysDec(&iter))
typedef struct{
    int i;//the number of entities we have passed that were not deleted.
    void* ptr;
    Entity ent;
    CompID component;//only used for ForList... functions
    Iter arrayIter;
    Component* comp;
}SysIter;
SysIter ForSysCreateIter(CompID component);
int ForSysTest(SysIter iter);
void ForSysDec(SysIter* iter);

SysIter ForListSysCreateIter(CompID component,List* lst);
int ForListSysTest(SysIter iter);
void ForListSysDec(SysIter* iter);

void ECSStartup();
CompID RegisterComponent(int typesize,componentInitFunc initFunc,componentDestroyFunc destroyFunc);
int IsEntityValid(Entity entity);
int CreateEntity();
void DestroyEntity(Entity entity);
void RemoveComponent(Entity entity,CompID component);
void* AddComponent(Entity entity,CompID component);
#define CallSystem(func,...) _CallSystem(func,__VA_ARGS__,-1)//I don't like -1. Components don't have negative
// values, which means that we *could* use unsigned numbers, that would make this bad.if we really wanted to we could
// have an 'index space' and a 'null space' like with entity ids.
void _CallSystem(SystemFunc func,CompID component,...);
void* GetComponent(Entity entity, CompID component);
void* GetComponentP(Entity entity,CompID component);// Use this if you don't know if the entity has said component
int HasComponent(Entity entity,CompID component);
