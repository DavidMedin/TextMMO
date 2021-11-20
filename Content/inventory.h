#pragma once
#include "../ecs.h"
#include "item.h"
CompID invID;
typedef struct{
    List entities;
    unsigned int maxCount;//max number of slots
        //0 == infinite because if you can't store anything,
        //get rid of the component.
    unsigned int count;//number of taken slots
}Inventory;
void InventoryInit(void* voidInv);
void AddToInv(Inventory* inv,Entity itemEID);
Entity RemoveFromInv(Inventory* inv,int index);
