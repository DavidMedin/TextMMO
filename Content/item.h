#pragma once
#include "../ecs.h"
CompID itemID;
typedef struct Item{
    int damage;//how much damage when wacking somebody/thing
    Entity owner;
    void(*onEquip) (struct Item*);
    void(*onDequip) (struct Item*);
    Link modHandle;
}Item;
void DealDamage(Entity defender,int damage);
void OnPickUp();
void ItemInit(void* rawItem);
