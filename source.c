#include "source.h"

void MeatBagInit(void* rawMeat){
    MeatBag* you = rawMeat;
    you->health = 100;
}
 void LookableInit(void* lookable){
    Lookable* look = lookable;
    look->name = "Unknown Thing";
    look->isVisible = 1;
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
    log_debug("Defered Destruction",entity);
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
int main(int argc,char** argv){
    setbuf(stdout,0);//bruh why do I have to do this?
    srand(time(NULL));

    FILE* logOut = fopen("log.txt","w");
    if(logOut==NULL)
        printf("Failed to open file!\n");
    log_add_fp(logOut,0);

    ECSStartup();

    deleteID = RegisterComponent(0,DeleteInit,NULL);
    lookID = RegisterComponent(sizeof(Lookable),LookableInit,NULL);
    humanID = RegisterComponent(sizeof(Humanoid),HumanoidInit,HumanoidDestroy);
    meatID = RegisterComponent(sizeof(MeatBag),MeatBagInit,NULL);
    itemID = RegisterComponent(sizeof(Item),ItemInit,NULL);
    aiID = RegisterComponent(sizeof(AI),AIInit,NULL);
    connID = RegisterComponent(sizeof(Connection),ConnectionInit,DestroyConnection);




    if(ServerInit()){
        return 1;
    }
    log_debug("started\n");

    nng_mtx_lock(mut);
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
    nng_mtx_unlock(mut);

    while(quitting != 1){
        nng_mtx_lock(mut);
        CallSystem(HumanConnUpdate,connID,humanID);
        CallSystem(TryLogin,connID);
        CallSystem(DeleteDefered,deleteID);
        nng_mtx_unlock(mut);
    }
    ServerEnd();
    fclose(logOut);

    return 0;
}