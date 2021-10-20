#pragma once
#include "ecs.h"
#include "source.h"
#include "lookable.h"
CompID humanID;
typedef struct{
    Entity hands[2];//can hold Items
    //char* name; if it is lookable, then use that name
}Humanoid;
void HumanConnUpdate(Entity ent);//System for Humanoid and Connection
void HumanAIUpdate(Entity ent);//System for Humanoid and AI
void HumanoidInit(void* human);
void HumanoidDestroy(void* humanVoid);
void PickUp(Entity picker,int hand,Entity pickee);
void DropItem(Entity dropper,int hand);
