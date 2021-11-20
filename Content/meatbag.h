#pragma once
#include "../ecs.h"

CompID meatID;
typedef struct{
    int health;//max 100
}MeatBag;
void MeatBagInit(void* rawMeat);
