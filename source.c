#include "source.h"
#include "GameActions.h"

/*
 * TODO: more game managing stuff (pick up item,drop item,list hands,drop stuff when you die, etc.)
 * TODO: Server Selection
 * TODO: Login (volatile data)
 * TODO: Log to file
 * TODO: System Mail System
 * TODO: Unity instruction compressing (look -> 0x5, reset -> 0x6)
 *      ^ Puts lets strain on network and server
 * TODO: async NPC enemies (attack every 5 seconds or whatever)
 */


void MeatBagInit(void* rawMeat){
    MeatBag* you = rawMeat;
    you->health = 100;
}
 void LookableInit(void* lookable){
    Lookable* look = lookable;
    look->name = "Unknown Thing";
}
void HumanoidInit(void* human){
    Humanoid* oid = human;
    oid->hands[0] = 0;//empty left hand
    oid->hands[1] = 0;//empty right hand
}
void ItemInit(void* rawItem){
    Item* item = rawItem;
    item->damage = 20;
    item->owner = 0;
}
void AIInit(void* rawAI){
    ((AI*)rawAI)->friendly = 0;
}
void DeleteInit(void* n){}
void DeleteDefered(Entity entity){
    printf("destroying entity {%d}\n",entity);
    DestroyEntity(entity);
}


void AIUpdate(Entity entity){
    //goes for all the AI entities
    AI* ai = GetComponent(entity,aiID);
    //Humanoid* humanoid = GetComponent(humanID,entity);
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
    //pick someone to attack
    if(attackList.count != 0) {
        tryAgain:;
        unsigned int attackIndex = rand() % ((unsigned int) attackList.count);
        For_Each(attackList, attackIter) {
            if (attackIter.i == attackIndex) {
                //Attack this person
                Entity defender = *(int *) attackIter.this->data;
                MeatBag *meat = GetComponent(defender,meatID);
                if (meat != NULL) {
                    if (meat->health <= 0) goto tryAgain;//this is simple. Don't @ me, John
                    Attack(entity, 1, defender);
                    break;
                }
            }
        }
    }

    FreeList(&attackList);
}

int quitting = 0;
int DoAction(Entity ent,char* line){
    List tokens = Listify(line);
    if(tokens.count != 0) {
        char *action = tokens.start->data;
        if (strcmp(action, "attack") == 0) {
            AttackString(ent, tokens);
            return 0;
        } else if (strcmp(action, "look") == 0) {
            Look(ent);
            return 0;
        } else if (strcmp(action, "spawn") == 0) {
            SpawnGoblin();
            return 0;
        }else if(strcmp(action, "help") == 0){
            //print some helpful stuff
            Sendf(GetComponent(ent,connID),"Commands----------\n\t*look\t--look around\n\t*attack {enemy "
                                          "name}\t--attack that "
                   "bitch\n\t*pick up {item}\t--pick up the item\n\t*spawn\t--spawn a goblin (for testing stuff)"
                   "\n\t*help\t--you are "
                   "here\n\t*quit\t--imagine quiting, I can't\n\t*join\t--join back in after quitting.");
            return 0;
        }
        else if(strcmp(action, "pick")==0 && strcmp((char*)tokens.start->next->data,"up")==0){
            char* itemName = tokens.start->next->next->data;
            For_System(lookID,lookIter){
                Lookable* lookee = SysIterVal(lookIter,Lookable);
                if(strcmp(lookee->name,itemName)==0){
                    PickUp(ent,0,lookIter.ent);
                    goto done;
                }
            }
            //didn't find it
            Connection* conn = GetComponent(ent,connID);
            if(conn){
                Sendf(conn,"That item doesn't exist!");
            }
            done:;
            return 0;
        }
    }
    CallSystem(AIUpdate,humanID,aiID);
    return 1;
}
int main(int argc,char** argv){
    setbuf(stdout,0);//bruh why do I have to do this?
    srand(time(NULL));

    ECSStartup();

    deleteID = RegisterComponent(0,DeleteInit,NULL);
    lookID = RegisterComponent(sizeof(Lookable),LookableInit,NULL);
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
    AddComponent(sword,lookID);
    ((Item*) GetComponent(sword,itemID))->damage = 21;
    ((Lookable*) GetComponent(sword,lookID))->name = "Sword";


    Entity orcishSword = CreateEntity();
    AddComponent(orcishSword,itemID);
    ((Item*) GetComponent(orcishSword,itemID))->damage = 25;

    Entity orc = CreateEntity();
    AddComponent(orc,humanID);
    AddComponent(orc,meatID);
    AddComponent(orc,aiID);
    AddComponent(orc,lookID);
    Humanoid* orcHuman = GetComponent(orc,humanID);
    Lookable* orcLook = GetComponent(orc,lookID);
    orcLook->name = "the orc";
    orcHuman->hands[1] = orcishSword;

    while(quitting != 1){
        nng_mtx_lock(mut);
        //go through connections and read their actions
        For_System(connID,connIter){
            Connection* conn = connIter.ptr;
            if(conn->actions.count > 0){
                //do actions
                For_Each(conn->actions,actionIter){
                    char* msg = Iter_Val(actionIter,char);
                    char* old = malloc(strlen(msg)+1);//preserve message before strtok
                    strcpy(old,msg);
                    old[strlen(msg)]=0;
                    if(strcmp(msg,"quit")==0){
                        DestroyEntity(connIter.ent);
                        free(old);
                        break;
                    }else
                        if(DoAction(connIter.ent,msg)){
                            TellEveryone("%s",old);
                        }
                    free(old);
                    RemoveElement(&actionIter);
                }
            }
        }
        CallSystem(DeleteDefered,deleteID);
        //For_System(deleteID,deleteIter){
        //    DestroyEntity(deleteIter.ent);
        //}
        nng_mtx_unlock(mut);
    }
    ServerEnd();

    return 0;
}