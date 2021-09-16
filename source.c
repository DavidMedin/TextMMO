#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ecs.h"
#include "termInput.h"
//TODO: Add HasComponent function
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
typedef struct{
    char friendly;
}AI;

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
void AIInit(void* rawAI){
    ((AI*)rawAI)->friendly = 0;
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

/*
Entity foundEntity = 0;//Will be the LAST humanoid that has this name;
void WhoIs(int eID,void* searchVoid){//For Humanoids
    Humanoid* human = GetComponent(humanID,eID);
    char* search = searchVoid;
    char* token = strtok_s(NULL," ",&search);

    char* nameDe = malloc(strlen(human->name)+1);
    strcpy(nameDe,human->name);
    nameDe[strlen(human->name)]=0;
    char* nameContext;
    char* nameToken = strtok_s(nameDe," ",&nameContext);
    while(token != NULL){
        if(nameToken == NULL){
            foundEntity = eID;
            break;
        }
        if(strcmp(token,nameToken) != 0){
            break;
        }
        token = strtok_s(NULL," ",&search);
       nameToken = strtok_s(NULL," ",&nameContext);
    }
    if(nameToken == NULL){
        foundEntity = eID;
    }
    free(nameDe);
}
 */

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
void AIUpdate(Entity entity){

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


    //create some items
    for(int i = 0;i < 32;i++){
        Entity tmpItem = CreateEntity();
        AddComponent(tmpItem,itemID);
    }
    //For_System(itemID,itemIter){
    //    printf("damage: %d\ti: %d\te: %d\n",((Item*)itemIter.ptr)->damage,itemIter.i,itemIter.ent);
    //}

    Attack(human,1,orc);
    while(1){
        //player turn
        int size;
        char* line = GetLine(&size);
        char* context;
        char* token = strtok_s(line," ",&context);
        if(token != NULL && strcmp(token,"attack") == 0){
            //char* defender = strtok_s(NULL," ",&context);
            //find the entity that cooresponds
            //CallSystem(WhoIs,humanID,context);
            Entity foundEntity = 0;
            For_System(humanID,humanoidIter){
                Humanoid* human = SysIterVal(humanoidIter,Humanoid);
                //search the name
                //create copy of name string
                char* delimitName = malloc(strlen(human->name)+1);
                delimitName[strlen(human->name)] = 0;
                strcpy(delimitName,human->name);
                //delimit it
                char* nameContext;
                char* phraseContext = context;
                char* phraseTok = strtok_s(NULL," ",&phraseContext);
                char* nameTok = strtok_s(delimitName," ",&nameContext);
                //start matching words
                while(strcmp(phraseTok,nameTok) == 0){
                    nameTok = strtok_s(NULL," ",&nameContext);
                    phraseTok = strtok_s(NULL," ",&phraseContext);
                    if(nameTok == NULL){
                        //if the we run out of source tokens first, it is a success
                        foundEntity = humanoidIter.ent;
                        goto exit;
                    }if(phraseTok == NULL){
                        goto exit;//not found
                    }
                }
            }
            exit:;
            if(foundEntity != 0){
                //found the entity
                Attack(human,1,foundEntity);
                if(((MeatBag*)GetComponent(meatID,foundEntity))->health <= 0){
                    printf("Knockout!\n");
                    free(line);
                    break;
                }
            }
        }
        free(line);
    }
    leave:;
/*
 *  While -Game loop
 *      get input
 *      process what it means -> switch; very basic
 *      call system based on what it is
 *          EX: Player attacks Orc. Call Players attack function -> calls Orc's damage function
 */

    return 0;
}