#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
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
	size_t size;//used for allocation.
	void* data;
	componentInitFunc initFunc;
}Component;
Component componentType[MAX_COMPONENTS];//Describes the component types (max 8)

char entities[MAX_COMPONENTS][MAX_ENTITIES];//describes what components belong to the entity

int RegisterComponent(int typesize,componentInitFunc initFunc){
	static int nextComponent = 0;
	if(nextComponent == MAX_COMPONENTS){
		printf("Reached max component capacity!\n");
		return -1;
	}
	componentType[nextComponent] = (Component){typesize,malloc(typesize*MAX_ENTITIES),initFunc};
	//components[nextComponent] = malloc(typesize*MAX_ENTITIES);
	return nextComponent++;
}
int CreateEntity(){
	static int nextEntity = 0;
	return nextEntity++;
}
void DestroyEntity(int entityID){
	memset(entities[entityID],0,MAX_COMPONENTS);
	printf("destroyed entity!\n");
}
void AddComponent(int entityID,int componentID){
	entities[entityID][componentID] = 1;
	Component* comp = &componentType[componentID];
	void* data = (char*)comp->data + comp->size * entityID;
	comp->initFunc(data);
}

typedef void(*SystemFunc)(int);//int is the entity ID.
void CallSystem(SystemFunc func,int componentID){
	for(int e = 0;e < MAX_ENTITIES;e++){
		//go through entities
		if(entities[e][componentID] == 1){
			//this entity has this component
			func(e);
		}
	}
}
void* GetComponent(int componentID,int entityID){
	return (char*)componentType[componentID].data + componentType[componentID].size * entityID;
}

//------------------
int humanID;
typedef struct{
	int health;
	float height;
}Human;
void HumanInit(void* component){
	Human* hum= component;
	hum->health = 100;
	hum->height = 5.0f + 9.0f/12.0f;
	printf("Initialized the Human!\n");
}
void HumanUpdate(int entityID){
	Human* human = GetComponent(humanID,entityID);
	printf("This human is %.2f meters tall\n",human->height);
}



int main(int argc,char** argv){
	humanID = RegisterComponent(sizeof(Human),HumanInit);
	if(humanID == -1) return 0;

	int dave = CreateEntity();
	AddComponent(dave,humanID);
	int jim  = CreateEntity();
	AddComponent(jim ,humanID);
	((Human*)GetComponent(humanID,jim))->height = 1;

	CallSystem(HumanUpdate,humanID);

	DestroyEntity(dave);

	CallSystem(HumanUpdate,humanID);

	return 0;
}