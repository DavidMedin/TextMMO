#pragma once
#include "../ecs.h"
CompID lookID;
typedef struct{
    char* name;
    char isVisible;
}Lookable;
void Look(Entity looker);
Entity Look_StringToEntity(char* string);
void LookableInit(void* lookable);
