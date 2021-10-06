#include "source.h"
#include "GameActions.h"

/*
 * TODO: Deconstruct Components (for Connections)
 * TODO: conns Connections list to ECS
 * TODO: more game managing stuff (reset, etc.)
 * TODO: multiplayer
 * TODO: Unity instruction compressing (look -> 0x5, reset -> 0x6)
 *      ^ Puts lets strain on network and server
 * TODO: async NPC enemies (attack every 5 seconds or whatever)
 */


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

int quitting = 0;
void DoAction(Entity ent,char* line){
    List tokens = Listify(line);
    if(tokens.count != 0) {
        char *action = tokens.start->data;
        if (strcmp(action, "attack") == 0) {
            AttackString(ent, tokens);
        } else if (strcmp(action, "look") == 0) {
            Look(ent);
            return;
        } else if (strcmp(action, "spawn") == 0) {
            SpawnGoblin();
        }else if(strcmp(action, "help") == 0){
            //print some helpful stuff
            Sendf(GetComponent(connID,ent),"Commands----------\n\t*look\t--look around\n\t*attack {enemy "
                                          "name}\t--attack that "
                   "bitch\n\t*spawn\t--spawn a goblin (for testing stuff)\n\t*help\t--you are "
                   "here\n\t*quit\t--imagine quiting, I can't");
            return;
        }
    }
    CallSystem(AIUpdate,humanID,aiID);
}
int main(int argc,char** argv){
    setbuf(stdout,0);//bruh why do I have to do this?
    srand(time(NULL));

    ECSStartup();

    humanID = RegisterComponent(sizeof(Humanoid),HumanoidInit,NULL);
    meatID = RegisterComponent(sizeof(MeatBag),MeatBagInit,NULL);
    itemID = RegisterComponent(sizeof(Item),ItemInit,NULL);
    aiID = RegisterComponent(sizeof(AI),AIInit,NULL);
    connID = RegisterComponent(sizeof(Connection),ConnectionInit,DestroyConnection);

    if(ServerInit()){
        return 1;
    }
    printf("started\n");

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

    while(quitting != 1){
        //DestroyWaiting();
        nng_mtx_lock(mut);
        //go through connections and read their actions
        List defer = {0};
        For_System(connID,connIter){
            Connection* conn = connIter.ptr;
            if(conn->actions.count > 0){
                //do actions
                For_Each(conn->actions,actionIter){
                    char* msg = Iter_Val(actionIter,char);
                    if(strcmp(msg,"quit")==0){
                        Entity* basket = malloc(sizeof(Entity));
                        *basket = connIter.ent;
                        PushBack(&defer,basket,1);
                        //DestroyEntity(connIter.ent);
                        break;
                    }else
                        DoAction(connIter.ent,msg);
                    RemoveElement(&actionIter);
                }
            }
        }
        For_Each(defer,deferIter){
            DestroyEntity(*Iter_Val(deferIter,Entity));
            RemoveElement(&deferIter);
        }
        nng_mtx_unlock(mut);
    }
    ServerEnd();

    return 0;
}