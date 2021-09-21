#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <raylib.h>
#include <nng/nng.h>

#include "ecs.h"
#include "termInput.h"
//TODO: think about events

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
        MeatBag* defenderMeat = GetComponent(meatID,defender);
        if(defenderMeat != NULL){
            Item* wackItem = GetComponent(itemID,attackHuman->hands[hand]);
            if(wackItem != 0){
                //there is an item in hand! Wack time
                //MeatBag* defenderMeat = GetComponent(meatID,defender);
                DealDamage(defender,wackItem->damage);
                printf("%s delt %d to %s and now %s has %d health!\n",attackHuman->name,wackItem->damage,
                       defenderHuman->name,defenderHuman->name,defenderMeat->health);
            }else{
                //printf("Item doesn't have the Item component!\n");
                //this guy is punching the other guy
                DealDamage(defender,5);
                printf("%s punched %s for %d and now %s has %d health!\n",attackHuman->name,
                       defenderHuman->name,5,defenderHuman->name,defenderMeat->health);
            }
            if(defenderMeat->health <= 0){
                printf("Knockout!\n");
                DestroyEntity(defender);
            }
        }else{

            printf("defender doesn't have a meatbag component!\n");
            return;
        }
    }else{
        printf("Either attacker or defender doesn't have a Humanoid component!\n");
    }
}

void AttackString(Entity attacker,List tokens){
    //find the entity that cooresponds
    For_System(humanID,humanoidIter){
        Humanoid* human = SysIterVal(humanoidIter,Humanoid);
        //search the name
        //create copy of name string
        char* delimitedName = malloc(strlen(human->name)+1);
        strcpy(delimitedName,human->name);
        delimitedName[strlen(human->name)] = 0;
        List nameList = Listify(delimitedName);
        Iter nameIter = MakeIter(&nameList);
        Iter tokenIter = MakeIter(&tokens);
        Inc(&tokenIter);//skip the action token
        while(1){
            int tok = Inc(&tokenIter);
            int nam = Inc(&nameIter);
            if(nam == 0) {
                break;//matched all name tokens
            }
            // }
            if(tok == 0) break;//didn't match all name tokens
           //check if they are the same
           if(strcmp(nameIter.this->data,tokenIter.this->data)!=0){
              //these are not the same
              break;
           }
        }
        //foundEntity = humanoidIter.ent;
        free(delimitedName);
    }
}

void AIUpdate(Entity entity){
    //goes for all the AI entities
    AI* ai = GetComponent(aiID,entity);
    Humanoid* humanoid = GetComponent(humanID,entity);
    //attack anything that is 'friendly' or doens't have an AI component but is a humanoid
    List attackList = {0};
    For_System(aiID,aiIter){
        AI* testAI = aiIter.ptr;
        if(testAI->friendly != ai->friendly){//these guys are enemies
            Entity* tmpEnt = malloc(sizeof(Entity));
            *tmpEnt = aiIter.ent;
            PushBack(&attackList,tmpEnt,sizeof(Entity));
        }
    }
    if(ai->friendly == 0){
        Entity* tmpEnt = malloc(sizeof(Entity));
        *tmpEnt = character;
        PushBack(&attackList,tmpEnt,sizeof(Entity));
    }

    //pick someone to attack
    tryAgain:;
    unsigned int attackIndex = rand() % ((unsigned int)attackList.count);
    For_Each(attackList,attackIter){
        if(attackIter.i == attackIndex){
            //Attack this person
            Entity defender = *(int*)attackIter.this->data;
            MeatBag* meat = GetComponent(meatID,defender);
            if(meat != NULL){
                if(meat->health <= 0) goto tryAgain;//this is simple. Don't @ me, John
                Attack(entity,1,defender);
                break;
            }
        }
    }

    FreeList(&attackList);
}


int main(int argc,char** argv){
    setbuf(stdout,0);//bruh why do I have to do this?
    srand(time(NULL));
    ECSStartup();

    humanID = RegisterComponent(sizeof(Humanoid),HumanoidInit);
    meatID = RegisterComponent(sizeof(MeatBag),MeatBagInit);
    itemID = RegisterComponent(sizeof(Item),ItemInit);
    aiID = RegisterComponent(sizeof(AI),AIInit);

    //create entities

    Entity sword = CreateEntity();
    AddComponent(sword,itemID);
    ((Item*) GetComponent(itemID,sword))->damage = 21;

    Entity orcishSword = CreateEntity();
    AddComponent(orcishSword,itemID);
    ((Item*) GetComponent(itemID,orcishSword))->damage = 25;

    Entity orc = CreateEntity();
    AddComponent(orc,humanID);
    AddComponent(orc,meatID);
    AddComponent(orc,aiID);
    Humanoid* orcHuman = GetComponent(humanID,orc);
    orcHuman->name = "the orc";
    orcHuman->hands[1] = orcishSword;
    //((Humanoid*) GetComponent(humanID,orc))->name = "the orc";

    Entity human = CreateEntity();
    character = human;
    AddComponent(human,humanID);AddComponent(human,meatID);
    Humanoid* humanHuman = GetComponent(humanID,human);
    humanHuman->name = "Jimmy";
    humanHuman->hands[1] = sword;

    printf("type 'help' for help, I guess.\n");
    while(1){
        //player turn
        int size;
        char* line = GetLine(&size);
        List tokens = Listify(line);
        if(tokens.count != 0) {
            char *action = tokens.start->data;
            if (strcmp(action, "attack") == 0) {
                AttackString(human, tokens);
            } else if (strcmp(action, "look") == 0) {
                //print all entities that isn't you
                printf("You look around and see");
                int found = 0;
                For_System(humanID, humanoidIter) {
                    Humanoid *humanComp = SysIterVal(humanoidIter, Humanoid);
                    if (humanoidIter.ent != human) {
                        //this isn't us
                        printf(" %s,", humanComp->name);
                        found = 1;
                    }
                }
                if (found == 0) {
                    printf("nobody");
                } else
                    printf("\b");
                printf(".\n");
                free(line);
                continue;//skip enemy turn
            } else if (strcmp(action, "spawn") == 0) {
                //spawn a goblin
                printf("a wild goblin appears!\n");
                Entity goblin = CreateEntity();
                AddComponent(goblin, humanID);
                ((Humanoid *) GetComponent(humanID, goblin))->name = "the goblin";
                AddComponent(goblin, meatID);
                AddComponent(goblin,aiID);
            }else if(strcmp(action, "help") == 0){
                //print some helpful stuff
                printf("Commands----------\n\t*look\t\t--look around\n\t*attack {enemy name}\t\t--attack that "
                       "bitch\n\t*spawn\t\t--spawn a goblin (for testing stuff)\n\t*help\t\t--you are "
                       "here\n\t*quit\t\t--imagine quiting, I can't\n");
                free(line);
                continue;//skip enemy turn
            }else if(strcmp(action,"quit") == 0){
                printf("You are a coward. Big yikes my dude.\n");
                free(line);
                break;
            }
        }
        free(line);
        CallSystem(AIUpdate,humanID,aiID);
    }

    return 0;
}