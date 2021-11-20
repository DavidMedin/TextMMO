#pragma once
#include "../ecs.h"
#include "../vec.h"
CompID deckID;
typedef struct Deck{
    Entity owner;
    Vec cards;//The card references that are in the deck.
    int num;//Number of actual cards in the deck.
    int next;//The next card to draw (index).
}Deck;
void DeckInit(void* nullDeck);
void DrawCard(Deck* deck);
void DeckAdd(Deck* deck,Entity card);//Add this card to the deck
