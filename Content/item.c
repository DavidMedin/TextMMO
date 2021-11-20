#include "item.h"
#include "ecs.h"
#include "source.h"
void ItemInit(void* rawItem){
    Item* item = rawItem;
    item->damage = 20;
    item->owner = 0;
    item->onEquip = 0;
    item->onDequip = 0;
    item->modHandle = 0;
}
void DealDamage(Entity defender,int damage){
    MeatBag* meat = GetComponent(defender,meatID);
    if(meat != NULL){
        meat->health -= damage;
    }else{
        log_error("defender {E: %d - V: %d} doesn't have a MeatBag component {%d}!",ID(defender),VERSION(defender),
                  meatID);
        return;
    }
}
