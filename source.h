#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#include "ecs.h"
#include "vec.h"
#include "termInput.h"
#include <server.h>
#include "memManager.h"
Entity character;
int meatID,humanID,itemID,aiID,connID;
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
