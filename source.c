#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ecs.h"
//TODO: another thing to do is create unit tests. IDK how but it is perhaps a good idea
/*
int humanID;
typedef struct{
	int health;
	float height;
    char* name;
}Human;

void HumanInit(void* component){
	Human* hum= component;
	hum->health = 100;
	hum->height = 5.0f + 9.0f/12.0f;
    hum->name = "Joe";
	printf("Initialized the Human!\n");
}
void HumanUpdate(int entityID){
	Human* human = GetComponent(humanID,entityID);
    if(!human){printf("Human update failed!\n");return;}
	printf("%s is %.2f meters tall\n",human->name,human->height);
}

int talkID;
typedef struct{
    char interactKey;
}Talkable;

void TalkableInit(void* component){
    Talkable* talk = component;
    talk->interactKey = 'e';
}
void InteractSystem(int eID){
    Talkable* talk = GetComponent(talkID,eID);
    printf("did you hit the %c key?\n",talk->interactKey);
}
*/

int inventoryID,itemID;
typedef struct{
    Entity item;
}Inventory;
typedef struct{
    char* name;
}Item;
void InventoryInit(void* rawInv){
    Inventory* inv = rawInv;
    inv->item = 0;
}
void PrintInventory(int entity){
    Inventory* inv = GetComponent(inventoryID,entity);
    if(inv->item != 0) {
        Item *containedItem = GetComponent(itemID, inv->item);
        if (containedItem != NULL) {
            printf("This inventory contains a %s!\n", containedItem->name);
        } else {
            inv->item = 0;//NULL
        }
    }
}
void ItemInit(void* rawItem){
    Item* item = rawItem;
    item->name = "Sword";//default value
}

void PrintPoolStuff(){
    printf("---------------\n");
    For_Each(components,iter){
        Component* comp = Iter_Val(iter,Component);
        unsigned int packedCount = comp->data.packed.list.count;
        unsigned int sparseCount = comp->data.sparse.list.count;
        printf("component %d :\n\tpacked count: %d\n",iter.i,packedCount);
        printf("\tsparse count: %d\n",sparseCount);
    }
    printf("deleted count : %d\n",deleted.list.count);
    printf("versions count : %d\n",versions.list.count);
    printf("--------------\n");
}
int main(int argc,char** argv){
    setbuf(stdout,0);//bruh why do I have to do this?
    ECSStartup();
    inventoryID = RegisterComponent(sizeof(inventoryID),InventoryInit);
    itemID = RegisterComponent(sizeof(Item),ItemInit);

    Entity item = CreateEntity();
    Entity inv = CreateEntity();
    AddComponent(item, itemID);
    AddComponent(inv,inventoryID);

    ((Inventory*)GetComponent(inventoryID,inv))->item = item;
    ((Item*) GetComponent(itemID,item))->name = "potion (++str)";

    CallSystem(PrintInventory,inventoryID);
    DestroyEntity(item);
    CallSystem(PrintInventory,inventoryID);
/*
 *  While -Game loop
 *      get input
 *      process what it means -> switch; very basic
 *      call system based on what it is
 *          EX: Player attacks Orc. Call Players attack function -> calls Orc's damage function
 */
    return 0;
}