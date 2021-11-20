#pragma once
#include "../ecs.h"
#include "lookable.h"
#include "item.h"

typedef void (*ModFunc)(void*,void*);
CompID humanID;
typedef struct{
    Entity hands[2];//can hold Items
    Entity deck;
    List cardHand;//List of all cards you can play
    List mods;//A list of functions to run on each card buffer
    /*
     +----------+----------------------+
     | Function | Abitrary data (args) |
     +----------+----------------------+
     */
    Link cardCaster;//links into mods
    Link cardTarget;

    int maxMana;
    int mana;
    Entity target;//who are we targeting?

    List view;//List of all entities you can see. 
   
    //char* name; if it is lookable, then use that name
}Humanoid;
void HumanConnUpdate(Entity ent);//System for Humanoid and Connection
void HumanAIUpdate(Entity ent);//System for Humanoid and AI
void HumanoidInit(void* human);
void HumanoidDestroy(void* humanVoid);
void PickUp(Entity picker,int hand,Entity pickee);
void DropItem(Entity dropper,int hand);
void AttackString(Entity attacker,List tokens);
void Attack(Entity attacker,int hand,Entity defender);
Link AddMod(Humanoid* human,ModFunc mod_func,int val);
