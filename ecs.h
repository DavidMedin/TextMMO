#pragma once
#include "packedSet.h"
#define ID(eID) (short)eID
#define VERSION(eID) ((short*)&eID)[1]
typedef void (*componentInitFunc)(void*);
typedef void (*componentDestroyFunc)(void*);
typedef void(*SystemFunc)(int);//int is the entity ID.
typedef int Entity;
typedef struct{
    PackedSet data;
    componentInitFunc initFunc;
    componentDestroyFunc destroyFunc;
}Component;

extern List components;
Pool deleted;
Pool versions;

#define For_System(component,iter) for(SysIter iter = ForSysCreateIter(component);ForSysTest(iter,component); \
ForSysInc(&iter, \
component))
#define SysIterVal(sysIter,type) ((type*)sysIter.ptr)
typedef struct{
    int i;
    void* ptr;
    Entity ent;
    Iter arrayIter;
    Component* comp;
}SysIter;
SysIter ForSysCreateIter(int compID);
int ForSysTest(SysIter iter,int componentID);
void ForSysInc(SysIter* iter,int componentID);

void ECSStartup();
int RegisterComponent(int typesize,componentInitFunc initFunc,componentDestroyFunc destroyFunc);
int IsEntityValid(int entity);
int CreateEntity();
void DestroyEntity(int entityID);
void AddComponent(int entityID,int componentID);
#define CallSystem(func,...) _CallSystem(func,__VA_ARGS__,-1)//I don't like -1. Components don't have negative
// values, which means that we *could* use unsigned numbers, that would make this bad.if we really wanted to we could
// have an 'index space' and a 'null space' like with entity ids.
void _CallSystem(SystemFunc func,int componentID,...);
void* GetComponent(int componentID,int entityID);
int HasComponent(Entity ent,int compID);