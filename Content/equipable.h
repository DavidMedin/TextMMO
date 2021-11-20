#pragma once
#include "../ecs.h"
CompID equipID;

typedef struct Equipable{
	Entity heldBy;//Entity that shold have a humanoid Component
	void(*onEquip) (struct Equipable*);
}Equipable;
void Equip_Init(void* equipNull);
