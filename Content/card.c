#include "card.h"
#include "../source.h"

void Card_Init(void* cardVoid){
    Card* card = cardVoid;
    card->propBuff = NULL;
    card->modPropBuff = NULL;
    card->use = NULL;
}

int Card_Use(Connection* conn,Card* card){
    char* msg = card->use(card);
    if(msg){
        // the card 'error'ed
        WriteByte(conn,usr_err);
        WriteOutput(conn,msg);
        WriteOutput(conn,", failed to cast the card");
        Send(conn);
        return 0;
    }
    return 1;
}

//{4,target,0,caster,0,heal,40,mana_cost,10}
char* Card_Heal(Card* card){
    int* buff = card->modPropBuff;
    Entity tar = buff[2];
    Entity player = buff[4];
    int heal = buff[6];
    int mana = buff[8];

    MeatBag* meat = GetComponent(tar,meatID);
    if(!meat)
        return "That entity doesn't have health!";
    Humanoid* caster = GetComponent(player,humanID);
    if(!caster)
        return "You aren't a humanoid!?";
    caster -= mana;
    meat->health += heal;

    Lookable* casterLook = GetComponent(player,lookID);
    Lookable* targetLook = GetComponent(tar,lookID);
    Connection* casterConn = GetComponentP(player,connID);
    Connection* targetConn = GetComponentP(tar,connID);
    if(!targetLook || !casterLook){
        //Panic like you're at a disco.
        if(!targetLook) log_fatal("A card was played against an entity {E: %d - V: %d} that didn't have a lookable component!",ID(tar),VERSION(tar));
        if(!casterLook) log_fatal("A card was played by an entity {E: %d - V: %d} that didn't have a lookable component!",ID(player),VERSION(player));
        return "An internal error occured (you or the entity you played a card against doesn't have a lookable component)";
    }
    if(casterConn) Sendf(casterConn,msg,"You healed %s for %d health, they have %d now.",targetLook->name,heal,meat->health);
    if(targetConn) Sendf(targetConn,msg,"%s healed you for %d health, you have %d now.",casterLook->name,heal,meat->health);
    return NULL;
}

//{4,target,0,caster,0,dmg,20,mana_cost,30}
char* Card_Slash(Card* card){
    int* buff = card->modPropBuff;
    Entity tar = buff[2];
    Entity player = buff[4];
    int damage = buff[6];
    int manaCost = buff[8];
    MeatBag* meat = GetComponentP(tar,meatID);
    if(!meat){
        return "That entity doesn't have health";
    }
    meat->health -= damage;
    Humanoid* playerHuman = GetComponent(player,humanID);
    playerHuman->mana -= manaCost;
    Lookable* tarLook = GetComponent(tar,lookID);
    Connection* tarConn = GetComponentP(tar,connID);
    Lookable* playerLook = GetComponent(player,lookID);
    Connection* playerConn = GetComponentP(player,connID);
    if(!tarLook || !playerLook){
        //Panic like you're at a disco.
        if(!tarLook) log_fatal("A card was played against an entity {E: %d - V: %d} that didn't have a lookable component!",ID(tar),VERSION(tar));
        if(!playerLook) log_fatal("A card was played by an entity {E: %d - V: %d} that didn't have a lookable component!",ID(player),VERSION(player));
        return "An internal error occured (you or the entity you played a card against doesn't have a lookable component)";
    }
    if(tarConn) Sendf(tarConn,msg,"you got hit by %s and now have %d hp",playerLook->name,meat->health);
    if(playerConn)Sendf(playerConn,msg,"You attack %s for %d heath, they have %d now",tarLook->name,damage,meat->health);
    return NULL;
}
