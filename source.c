#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ecs.h"
//TODO: Add HasEntity function
//TODO: Add multi component system calls
//TODO: think about events

int meatID,humanID,itemID;
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

void MeatBagInit(void* rawMeat){
    MeatBag* you = rawMeat;
    you->health = 100;
}
void HumanoidInit(void* human){
    Humanoid* oid = human;
    oid->hands[0] = 0;//empty left hand
    oid->hands[1] = 0;//empty right hand
    oid->name = "a human";
}
void ItemInit(void* rawItem){
    Item* item = rawItem;
    item->damage = 20;
}
void DealDamage(Entity defender,int damage){
    MeatBag* meat = GetComponent(meatID,defender);
    if(meat != NULL){
        meat->health -= damage;
    }else{
        printf("defender doesn't have a MeatBag component!\n");
        return;
    }
}
void Attack(Entity attacker,int hand,Entity defender){
    //attacker is attacking defender with their {hand} hand!
    if(hand < 0 || hand > 1){
        printf("hand index out of range! %d \n",hand);
        return;
    }
    //Get Humanoid Component
    Humanoid* attackHuman = GetComponent(humanID,attacker);
    Humanoid* defenderHuman = GetComponent(humanID,defender);
    if(attackHuman != NULL && defenderHuman != NULL){
        Item* wackItem = GetComponent(itemID,attackHuman->hands[hand]);
        if(wackItem != 0){
            //there is an item in hand! Wack time
            MeatBag* defenderMeat = GetComponent(meatID,defender);
            if(defenderMeat != NULL){
                DealDamage(defender,wackItem->damage);
                printf("%s delt %d to %s and now %s has %d health!\n",attackHuman->name,wackItem->damage,
                       defenderHuman->name,defenderHuman->name,defenderMeat->health);
            }else{
                printf("defender doesn't have a meatbag component!\n");
                return;
            }
        }else{
            printf("Item doesn't have the Item component!\n");
        }
    }else{
        printf("Either attacker or defender doesn't have a Humanoid component!\n");
    }
}

int main(int argc,char** argv){
    setbuf(stdout,0);//bruh why do I have to do this?
    ECSStartup();

    humanID = RegisterComponent(sizeof(Humanoid),HumanoidInit);
    meatID = RegisterComponent(sizeof(MeatBag),MeatBagInit);
    itemID = RegisterComponent(sizeof(Item),ItemInit);

    //create entities

    Entity sword = CreateEntity();
    AddComponent(sword,itemID);
    ((Item*) GetComponent(itemID,sword))->damage = 21;

    Entity orc = CreateEntity();
    AddComponent(orc,humanID);
    AddComponent(orc,meatID);
    ((Humanoid*) GetComponent(humanID,orc))->name = "the orc";

    Entity human = CreateEntity();
    AddComponent(human,humanID);AddComponent(human,meatID);
    Humanoid* humanHuman = GetComponent(humanID,human);
    humanHuman->name = "Jimmy";
    humanHuman->hands[1] = sword;

    Attack(human,1,orc);
/*
 *  While -Game loop
 *      get input
 *      process what it means -> switch; very basic
 *      call system based on what it is
 *          EX: Player attacks Orc. Call Players attack function -> calls Orc's damage function
 */

    return 0;
}