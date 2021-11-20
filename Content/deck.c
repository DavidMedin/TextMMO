#include "deck.h"
#include "../deps/logger/log.h"
#include "humanoid.h"

void DeckInit(void* nullDeck){
    Deck* deck = nullDeck;
    deck->cards = VecMake(sizeof(Entity),20);
    // 20 is how many card references to allocate.

    deck->next = 0;
    deck->owner = 0;
    deck->num = 0;
}

void DeckAdd(Deck* deck,Entity card){
    if(deck->num++ == 0)
        *(Entity*)VecLast(&deck->cards) = card;
    else
        *(Entity*)VecNext(&deck->cards) = card;
}


void DrawCard(Deck* deck){
    if(deck->owner == 0){
        log_fatal("DrawCard was called without a humanoid owner!");
        return;
    }
    Humanoid* human = GetComponent(deck->owner,humanID);
    Entity nextCard = *(Entity*)VecGet(deck->cards,deck->next);
    PushBack(&human->cardHand,CreateBasket(sizeof(Entity),&nextCard),sizeof(Entity));
    deck->next =(deck->next+1) % deck->num;
}
