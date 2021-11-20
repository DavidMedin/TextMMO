//
// Created by david on 10/29/21.
//

#include "inventory.h"
#include "log.h"
#include "lookable.h"

void InventoryInit(void* voidInv){
    Inventory* inv = voidInv;
    inv->count = 10;
    inv->maxCount = 0;

    inv->entities.count = 0;
    inv->entities.end = NULL;
    inv->entities.start = NULL;
}
void AddToInv(Inventory* inv,Entity itemEID){
    Item* item = GetComponent(itemEID,itemID);
    if(item->owner) {
        log_error("Attempted to put owned item {E: %d - V: %d} into an inventory", ID(itemID), VERSION(itemID));
        return;
    }
    PushBack(&inv->entities, CreateBasket(sizeof(Entity),&itemEID),sizeof(Entity));
    item->owner = *ENTFROMCOMP(inv);
    Lookable* look = GetComponent(itemEID,lookID);
    if(look){
        look->isVisible = 0;
    }else{
        log_error("Item doesn't have a Lookable component. It should!");
    }
}
Entity RemoveFromInv(Inventory* inv,int index){
    For_List_System(itemID,inv->entities,itemIter){
        if(itemIter.i == index){
            Item* item = itemIter.ptr;
            item->owner = 0;
            Lookable* look = GetComponent(itemIter.ent,lookID);
            if(look){
                look->isVisible = 1;
            }
            RemoveElement(&itemIter.arrayIter);
            return itemIter.ent;
        }
    }
    return 0;
}
