#include "source.h"

void DeleteInit(void* n){}
void DeleteDefered(Entity entity){
    log_debug("Defered Destruction",entity);
    DestroyEntity(entity);
}

void DoubleDamage(void* argBuffer,void* val){
    int* intBuff = argBuffer;
    int size = *intBuff;
    for(int i = 0;i < size;i++){
        if(intBuff[i*2 + 1] == dmg){
            intBuff[i*2+2] *= 2;
        }
    }
}
void SwordEquip(Item* equip){
    Sendf(GetComponent(equip->owner,connID),msg,"You picked up this sword!");
    //add a mod
    equip->modHandle = AddMod(GetComponent(equip->owner,humanID),DoubleDamage,0);
}
void SwordDequip(Item* equip){
    Iter tmpIter = {0};
    tmpIter.last = equip->modHandle->last;
    tmpIter.next = equip->modHandle->next;
    tmpIter.this = equip->modHandle;
    tmpIter.root = equip->modHandle->root;
    RemoveElement(&tmpIter);
    Sendf(GetComponent(equip->owner,connID),msg,"You dropped this sword!");
}
int quitting = 0;
int main(int argc,char** argv){
    setbuf(stdout,0); ///bruh why do I have to do this?
    srand(time(NULL));

    FILE* logOut = fopen("log.txt","w");
    if(logOut==NULL)
        printf("Failed to open file!\n");
    log_add_fp(logOut,0);


    //Test vector stuff
    //Vec lst = VecMake(sizeof(int),5);
    //log_debug("last %d",lst.last);
    //*(int*)VecNext(&lst) = 2;
    //log_debug("last val %d",*(int*)VecLast(&lst));
    //*(int*)VecNext(&lst) = 5;
    //log_debug("next val %d", *(int*)VecLast(&lst));


    ECSStartup();

    deleteID = RegisterComponent(0,DeleteInit,NULL);
    lookID = RegisterComponent(sizeof(Lookable),LookableInit,NULL);
    humanID = RegisterComponent(sizeof(Humanoid),HumanoidInit,HumanoidDestroy);
    meatID = RegisterComponent(sizeof(MeatBag),MeatBagInit,NULL);
    itemID = RegisterComponent(sizeof(Item),ItemInit,NULL);
    connID = RegisterComponent(sizeof(Connection),ConnectionInit,DestroyConnection);
    invID = RegisterComponent(sizeof(Inventory),InventoryInit,NULL);
    cardID = RegisterComponent(sizeof(Card),Card_Init,NULL);
    deckID = RegisterComponent(sizeof(Deck),DeckInit,NULL);


    if(ServerInit()){
        return 1;
    }
    log_debug("started\n");


    nng_mtx_lock(mut);
    Entity sword = CreateEntity();
    Item* swordItem = AddComponent(sword,itemID);
    swordItem->damage = 22;
    swordItem->onEquip = SwordEquip;
    swordItem->onDequip= SwordDequip;
    ((Lookable*)AddComponent(sword,lookID))->name = "Sword";


    Entity orcishSword = CreateEntity();
    ((Item*) AddComponent(orcishSword,itemID))->damage = 25;
    ((Lookable*) AddComponent(orcishSword,lookID))->name = "Orcish_Sword";

    Entity orc = CreateEntity();
    AddComponent(orc,meatID);
    Humanoid* orcHuman = AddComponent(orc,humanID);
    Lookable* orcLook = AddComponent(orc,lookID);
    orcLook->name = "the orc";
    orcHuman->hands[1] = orcishSword;

    Entity double_sword = CreateEntity();
    ((Item*)AddComponent(double_sword,itemID))->damage = 69;
    ((Lookable*)AddComponent(double_sword,lookID))->name = "Double_Ended_Sword";

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
