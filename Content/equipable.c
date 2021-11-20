#include "equipable.h"

void Equip_Init(void* equipNull){
    Equipable* equip = equipNull;
    equip->heldBy = 0;
    equip->onEquip = NULL;
}
