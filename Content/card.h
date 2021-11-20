#pragma once
#include "../ecs.h"
#include "../server.h"

typedef enum{
    targetEnt,
    dmg,
    heal,
    manaCost,
    casterEnt,
}PropName;

CompID cardID;
typedef struct Card{ // Is a component
    int buffSize;
	void* propBuff;
	void* modPropBuff;
	char* (*use)(struct Card*);
}Card;
void Card_Init(void* cardVoid);
void Card_Deinit(void* cardVoid);//TODO
int Card_Use(Connection* conn,Card* card);

char* Card_Slash(Card* card);
char* Card_Heal(Card* card);
