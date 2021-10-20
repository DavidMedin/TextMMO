#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <log.h>

#include "ecs.h"
#include "vec.h"
#include "termInput.h"
#include <server.h>
#include "Content/humanoid.h"
#include "Content/lookable.h"
CompID deleteID,meatID,itemID,aiID,connID;
typedef struct{
    int health;//max 100
}MeatBag;
typedef struct{
    int damage;//how much damage when wacking somebody/thing
    Entity owner;
}Item;
typedef struct{
    char friendly;
}AI;
void DeleteDefered(Entity entity);
