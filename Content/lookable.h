#pragma once
CompID lookID;
typedef struct{
    char* name;
    char isVisible;
}Lookable;
void Look(Entity looker);
