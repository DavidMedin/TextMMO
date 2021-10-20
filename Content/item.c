//
// Created by DSU on 10/19/2021.
//

#include "ecs.h"
#include "source.h"
#include "item.h"
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
