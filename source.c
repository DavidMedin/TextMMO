#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ecs.h"

//------------------
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

void PrintPoolStuff(){
    printf("---------------\n");
    For_Each(components,iter){
        Component* comp = Iter_Val(iter,Component);
        int packedCount = comp->data.packed.list.count;
        int sparseCount = comp->data.sparse.list.count;
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
    PrintPoolStuff();
    humanID = RegisterComponent(sizeof(Human),HumanInit);
    printf("The human ID is %d\n",humanID);
    talkID = RegisterComponent(sizeof(Talkable),TalkableInit);
    printf("The talkable ID is %d\n",talkID);

    int jim = CreateEntity();
    printf("Jim's entity ID is %d, and his version is %d\n",(short)jim,((short*)(&jim))[1]);
    int dave = CreateEntity();
    printf("Dave's entity ID is %d, and his version is %d\n",(short)dave,((short*)(&dave))[1]);
    int shopkeeper = CreateEntity();
    printf("The shopkeeper's ID is %d, and his version is %d\n",(short)shopkeeper,((short*)(&shopkeeper))[1]);

    AddComponent(jim,humanID);
    AddComponent(dave,humanID);
    AddComponent(shopkeeper,humanID);
    AddComponent(dave,talkID);
    AddComponent(shopkeeper,talkID);
    PrintPoolStuff();
    Human* jimHuman = (Human*) GetComponent(humanID,jim);
    jimHuman->name = "Jim";
    Human* daveHuman = (Human*) GetComponent(humanID,dave);
    daveHuman->name = "Dave";


    CallSystem(HumanUpdate,humanID);
    CallSystem(InteractSystem,talkID);

    DestroyEntity(dave);
    DestroyEntity(jim);
    DestroyEntity(shopkeeper);
    PrintPoolStuff();

    CallSystem(HumanUpdate,humanID);
    dave = CreateEntity();
    printf("Dave's entity ID is %d, and his version is %d\n",(short)dave,((short*)(&dave))[1]);
    AddComponent(dave,humanID);
    AddComponent(dave,talkID);
    PrintPoolStuff();
    CallSystem(HumanUpdate,humanID);
    CallSystem(InteractSystem,talkID);

    return 0;
}