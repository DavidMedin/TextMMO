//
// Created by DSU on 10/3/2021.
//

#ifndef TEXTLICIOUS_SOURCE_H
#define TEXTLICIOUS_SOURCE_H

#endif //TEXTLICIOUS_SOURCE_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#include "ecs.h"
#include "vec.h"
#include "termInput.h"
#include <server.h>
Entity character;
int meatID,humanID,itemID,aiID;
typedef struct{
    int health;//max 100
}MeatBag;
typedef struct{
    Entity hands[2];//can hold Items
    char* name;
}Humanoid;
typedef struct{
    int damage;//how much damage when waking somebody/thing
}Item;
typedef struct{
    char friendly;
}AI;
