#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <log.h>

#include "ecs.h"
#include "vec.h"
#include "termInput.h"
#include <server.h>
Entity character;
CompID deleteID,lookID,meatID,humanID,itemID,aiID,connID;
typedef struct{
    char* name;
}Lookable;
typedef struct{
    int health;//max 100
}MeatBag;
typedef struct{
    Entity hands[2];//can hold Items
    //char* name; if it is lookable, then use that name
}Humanoid;
typedef struct{
    int damage;//how much damage when wacking somebody/thing
    Entity owner;
}Item;
typedef struct{
    char friendly;
}AI;
