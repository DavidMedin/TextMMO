//
// Created by DSU on 10/19/2021.
//

#include "humanoid.h"
#include "../source.h"
#include "log.h"
//#include "GameActions.h"
void LowerString(char* str){
    for(;*str;++str)*str=tolower(*str);
}
void LowerStringList(List lst){
    For_Each(lst,strIter){
	LowerString(strIter.this->data);
    }
}

void EnumHand(Entity ent,Connection* conn){
    //Sendf(conn,msg,"Enumerating in progress");
    Humanoid* human = GetComponent(ent,humanID);
    For_Each(human->cardHand,handIter){
        Lookable* card_look = GetComponent(*Iter_Val(handIter,Entity),lookID);
        Sendf(conn,msg,"\t[%d] %s",handIter.i,card_look->name);
    }
}
void Play(Entity ent,Connection* conn, int index){
    //Initialize modified buffer
//    Sendf(conn,msg,"'played' a card");
    Entity cardEnt = 0;
    Humanoid* human = GetComponent(ent,humanID);
    if(!human){
        log_error("a non human tried to play a card");
        return;
    }
    Iter cardIterLong;
    For_Each(human->cardHand,cardIter){
        if(cardIter.i == index){
            cardIterLong = cardIter;
            cardEnt = *Iter_Val(cardIter,Entity);
            break;
        }
    }
    Card* card = GetComponent(cardEnt,cardID);
    if(!card){
        log_error("Tried to pay a card that is not a card");
        return;
    }
    card->modPropBuff = malloc(card->buffSize);
    memcpy(card->modPropBuff,card->propBuff,card->buffSize); 

    //send it through the modifiers
    For_Each(human->mods,modIter){
        void* modBuff = modIter.this->data;
        //Giving to the mod function the modified property buffer and
        // the arguments for the mod function.
        (*(ModFunc*)modBuff)(card->modPropBuff,modBuff + sizeof(ModFunc));
    }

    //use card
    if(Card_Use(conn,card)){
        //succeded to play card
        //discard card
        RemoveElement(&cardIterLong);
        //This should be checked. {vv} Though it should never fail.
        DrawCard(GetComponent(human->deck,deckID));
    }
}

int UpdateHumanoid(Entity ent,char* line){
    List tokens = Listify(line);
    LowerStringList(tokens);
    Connection* conn = GetComponent(ent,connID);
    if(tokens.count != 0) {
        if(tokens.start == NULL){
            Sendf(conn,usr_err,"Please don't send null strings :p");
            log_error("User tried to send a null string");
            return 0;
        }
        char *action = tokens.start->data;
        if(strcmp(action, "play") == 0 ) {
            //play a card
            if(tokens.start->next == NULL){
                Sendf(conn,usr_err,"'Play' requires an argument (card index)");
                log_error("User tried to use 'Play' without an argument");
                return 0;
            }
            int num = atoi(tokens.start->next->data);
            //handle when 'num' is not a number.
            Play(ent,conn,num);
            return 0;
        }
        else if(strcmp(action, "target") == 0){
            if(tokens.start->next == NULL){
                Sendf(conn,usr_err,"'Target' requires an argument (look index)");
                log_error("User tried to use 'Target' without an argument");
                return 0;
            }
            char* numStr = tokens.start->next->data;
            int num = atoi(numStr);
            if(num != 0 || (num == 0 && *numStr == '0' || *(numStr+1) == 0)){// "00" is not a number
                Humanoid* human = GetComponent(ent,humanID);
                For_Each(human->view,viewIter){
                    human->target = *Iter_Val(viewIter,Entity);
                    //log_debug("Evaluting %d : %d",viewIter.i,human->target);
                    if(viewIter.i == num){
                        //Modify the mod
                        log_debug("Target set to %d",human->target);
                        *(int*)(human->cardTarget->data + sizeof(ModFunc)) = human->target;
                        return 0;
                    }
                }
                Sendf(conn,usr_err,"Tried to target {%d} an invalid entity. (Forgot to 'look'?)",num);
                log_error("User tried to target {%d} an invalid entity",num);
                return 0;
            }else{
                Sendf(conn,usr_err,"Target by name is currently unimplemented");
                log_error("User tried to target by name");
            }
            return 1;
        }
        else if (strcmp(action, "look") == 0) {
            Look(ent);
            //clear target
            ((Humanoid*)GetComponent(ent,humanID))->target = 0;
            return 0;
        }else if(strcmp(action, "help") == 0){
            //print some helpful stuff
            Sendf(conn,msg,"Commands----------\n\t*look\t--look around\n\t*attack {enemy "
                                           "name}\t--attack that "
                                           "bitch\n\t*pick up {item}\t--pick up the item\n\t*help\t--you are "
                                           "here\n\t*quit\t--imagine quiting, I can't\n\t*join\t--join back in after quitting.");
            return 0;
        }
        else if(strcmp(action, "pick")==0 && tokens.start->next && strcmp((char*)tokens.start->next->data,"up")==0) {
            //if(tokens.start->next == 0){
		//log_error("User tried to invoke 'pick'");
		//Sendf(conn,usr_err,"Did you mean 'pick up'?");
		//return 0;
            //}
            if(tokens.start->next->next == 0){
                log_error("User didn't give arguments for 'pick up'");
                Sendf(conn,usr_err,"'pick up' expects one argument (what item to pick up)");
                return 0;
            }
            char* itemName = tokens.start->next->next->data;
            //drop this into it's inventory
            Inventory* inv = GetComponent(ent,invID);
            int num = atoi(itemName);
            if(inv){
                if(num != 0 || (num == 0 && *itemName == '0' || *(itemName+1) == 0)){// "00" is not a number
                    //Is a number
                    Humanoid* human = GetComponent(ent,humanID);
                    Entity* viewEnt = GetNth(&human->view,num);
                    if(viewEnt){
                        AddToInv(inv,*viewEnt);
                        Lookable* itemLook = GetComponent(*viewEnt,lookID);
                        if(conn) Sendf(conn,msg,"You picked up %s",itemLook->name);
                    }else{
                        log_error("User tried to pick up an entity {N : %d} that is out of bounds of their view",num);
                        Sendf(conn,usr_err,"Failed to pick up out of bounds entity!");
                        return 0;
                    }
                }else{
                    //Is a string
                    Entity itemE = Look_StringToEntity(itemName);
                    if(itemE){
                        AddToInv(inv,itemE);
                        if(conn) Sendf(conn,msg,"You picked up %s",itemName);
                    }else if(conn){
                        Sendf(conn,usr_err,"That item doesn't exist!");
                    }
                }
            }else if(conn){
                Sendf(conn,usr_err,"You don't have an inventory!? What?");
            }
            return 0;
        }
        else if(strcmp(action,"equip")==0){
            // equip Sword -> Equip the Sword from the floor
            // equip i 2 -> equip the 3rd item from the inventory
            if(!tokens.start->next){
                if(conn) Sendf(conn,usr_err,"No inventory name or item number specified.");
                log_error("User didn't specify inventory or item number");
                return 0;
            }
            char* itemName = tokens.start->next->data;
            int num = atoi(itemName);
            if(num != 0 || (num == 0 && *itemName == '0' || *(itemName+1) == 0)){// "00" is not a number
                //is a number
                    Humanoid* human = GetComponent(ent,humanID);
                    Entity* viewEnt = GetNth(&human->view,num);
                    if(viewEnt){
                        PickUp(ent, 0, *viewEnt);
                    }else{
                        log_error("User tried to equip an entity {N : %d} that is out of bounds of their view",num);
                        Sendf(conn,usr_err,"Failed to equip out of bounds entity!");
                        return 0;
                    }
                    goto done;
            }
            else if(strlen(itemName) == 1){
                //is not a number
                if(*itemName == 'i') {
                    if (!tokens.start->next->next) {
                        if (conn) Sendf(conn, usr_err, "No item number specified");
                        log_error("User didn't specify item number");
                        return 0;
                    }
                    char *numStr = tokens.start->next->next->data;
                    int num = atoi(numStr);
                    if (num == 0 && *numStr != 48) {
                        if (conn) Sendf(conn, usr_err, "Equip by name is not currently supported");
                        log_error("User tried to equip from inventory by name, not number");
                        return 0;
                    }
                    Inventory *inv = GetComponent(ent, invID);
                    if (!inv) {
                        log_error("Humanoid doesn't have an inventory!?!");
                        return 0;
                    }
                    Humanoid *human = GetComponent(ent, humanID);
                    if (human->hands[0] != 0) {
                        if (conn) Sendf(conn, usr_err, "Right hand is full!");
                        log_error("User tried to equip item in a hand (right) that is full!");
                        return 0;
                    }
                    Entity item = RemoveFromInv(inv, num);
                    if (!item) {
                        if (conn) Sendf(conn, usr_err, "Invalid item id specified");
                        log_error("User tried to equip invalid item from inventory");
                        return 0;
                    }
                    PickUp(ent, 0, item);
                    return 0;
                }
            }else {
                For_System(lookID, lookIter) {
                    Lookable *lookee = SysIterVal(lookIter, Lookable);
                    char lowerName[strlen(lookee->name)+1];
                    lowerName[strlen(lookee->name)] = 0;
                    strcpy(lowerName,lookee->name);
		    LowerString(lowerName);
                    if (strcmp(lowerName, itemName) == 0) {
                        PickUp(ent, 0, lookIter.ent);
                        goto done;
                    }
                }
            }
            //didn't find it
            if(conn){
                Sendf(conn,usr_err,"That item doesn't exist!");
            }
            done:;
            return 0;
        }else if(strcmp(action,"drop")==0){
            if(tokens.start->next == NULL){
                if(conn) Sendf(conn,usr_err,"What thing do you want to drop?");
                log_error("User didn't specify what to drop");
                return 0;
            }
            char* what = tokens.start->next->data;
            if(*what == 'r'){
                DropItem(ent,0);
            }else if(*what == 'l') {
                DropItem(ent,1);
            }else if(*what == 'i'){
                //get what item to drop
                Inventory* inv = GetComponent(ent,invID);
                if(!inv){
                    if(conn) Sendf(conn,usr_err,"You don't have an inventory!");
                    log_error("User tried to drop something from an inventory they didn't have!");
                    return 0;
                }
                if(tokens.start->next->next == NULL){
                    if(conn) Sendf(conn,usr_err,"What item do you want to drop?");
                    log_error("User didn't specify what item to drop");
                    return 0;
                }
                char* numStr = tokens.start->next->next->data;
                int num = atoi(numStr);
                Entity item = RemoveFromInv(inv,num);
                if(!item) {if(conn) Sendf(conn,usr_err,"Index {%d} in inventory is out of bounds!",num);return 0;}
                Lookable* itemLook = GetComponent(item,lookID);
                char* name = itemLook->name ? itemLook->name : "something";
                if(conn) Sendf(conn,msg,"You dropped %s from your inventory!",name);
                itemLook->isVisible = 1;
            }
            return 0;
        }else if(strcmp(action,"show")==0){
            //what to show?
            if(tokens.start->next == 0){
                log_error("User tried to use 'show' without any arguments");
                Sendf(conn,usr_err,"'show' requires one argument!");
                return 0;
            }
            char* what = tokens.start->next->data;
            if(*what == 'i' || strcmp(what,"inv")==0 || strcmp(what,"inventory")==0){
                ///print out the contents of an inventory
                Inventory* inv = GetComponent(ent,invID);
                if(inv && conn){
                    For_List_System(lookID,inv->entities,itemEntIter){
                        //Lookable* look = SysIterVal(itemIter,Lookable);
                        Lookable* look = SysIterVal(itemEntIter,Lookable);
                        Sendf(conn,msg,"\t(%d) %s",itemEntIter.i,look->name);
                    }
                }else{
                    log_error("entity {E: %d - V: %d} tried to enumerate inventory, but doesn't have one!",ID(ent),
                              VERSION(ent));
                if(conn){
                    Sendf(conn,usr_err,"You don't have an inventory!");
                }}
                return 0;
            }else if(*what == 'e'){
        		//show what is equipped
        		if(conn){
            			Humanoid* human = GetComponent(ent,humanID);
            			//assume human exists. idc.
            			Sendf(conn,msg,"Equiped items");
            			char* rightName = human->hands[0] ? ((Lookable*)GetComponent(human->hands[0],lookID))->name : "nothing";//name of the item in the right hand
        			Sendf(conn,msg,"Right hand: %s",rightName);

            			char* leftName = human->hands[1] ? ((Lookable*)GetComponent(human->hands[1],lookID))->name : "nothing";//name of the item in the left hand
        			Sendf(conn,msg,"Right hand: %s",leftName);
        		}
        		return 0;
            }else if(*what == 'h'){
                //show hand
                if(conn){
                    EnumHand(ent,conn);
                }
                return 0;
            }
        }
    }
    return 1;
}
extern int quitting;
void HumanConnUpdate(Entity ent) {//System for Humanoid and Connection
    Connection *conn = GetComponent(ent, connID);
    if (conn->actions.count > 0) {
        //do actions
        For_Each(conn->actions, actionIter) {
            char *message = Iter_Val(actionIter, char);
            char *old = malloc(strlen(message) + 1);//preserve message before strtok
            strcpy(old, message);
            old[strlen(message)] = 0;
            if (strcmp(message, "quit") == 0) {
                AddComponent(ent, deleteID);
            	RemoveElement(&actionIter);
                free(old);
                break;
            } else if (UpdateHumanoid(ent, message)) {
                TellEveryone(msg, "%s", old);
            }
            free(old);
            RemoveElement(&actionIter);
        }
    }
}

void Mod_Target(void* modBuff,void* argBuff){
    log_debug("Running Mod_Target modification");
    int* args = modBuff;
    int size = args[0];
    for(int i = 0;i < size;i++){
        if(args[i * 2+ 1] == targetEnt){
            log_debug("Setting target to %d",*(int*)argBuff);
            args[i * 2 + 2] = *(int*)argBuff;
            break;
        }
    }
}

void Mod_Caster(void* modBuff,void* argBuff){
    log_debug("Running Mod_Caster modification");
    int* args = modBuff;
    int size = args[0];
    for(int i = 0;i < size;i++){
        if(args[i * 2+ 1] == casterEnt){
            args[i * 2 + 2] = *(int*)argBuff;
            break;
        }
    }
}

Link AddMod(Humanoid* human,ModFunc mod_func,int val){
    void* mod = malloc(sizeof(ModFunc)+sizeof(int));
    *(ModFunc*)mod= mod_func;
    *(Entity*)(mod + sizeof(ModFunc)) = val;
    return PushBack(&human->mods,mod,sizeof(ModFunc)+sizeof(int));
}

//void HumanAIUpdate(Entity ent);//System for Humanoid and AI
void HumanoidInit(void* human){
    Humanoid* oid = human;
    oid->mods.count = 0;
    oid->mods.end = NULL;
    oid->mods.start = NULL;
    oid->hands[0] = 0;//empty left hand
    oid->hands[1] = 0;//empty right hand

    oid->maxMana = 50;
    oid->mana = oid->maxMana;
    oid->target = 0;

    oid->cardCaster = NULL;
    oid->cardTarget = NULL;

    oid->view.count = 0;
    oid->view.end = 0;
    oid->view.start = 0;

    // Give all humanoids a card
    oid->cardHand.count = 0;
    oid->cardHand.end = 0;
    oid->cardHand.start = NULL;


    //create the new card

    Entity slCard = CreateEntity();
    Item* slItemCard = AddComponent(slCard,itemID);
    Lookable* slLook = AddComponent(slCard,lookID);
    Card* slCardCard = AddComponent(slCard,cardID);

    int slBuffCpy[] = {4,targetEnt,0,casterEnt,0,dmg,20,manaCost,20};// 
    slItemCard->owner = *ENTFROMCOMP(oid);
    slCardCard->use = Card_Slash;

    slCardCard->propBuff = malloc(sizeof(slBuffCpy));
    memcpy(slCardCard->propBuff,slBuffCpy,sizeof(slBuffCpy));
    slCardCard->buffSize = sizeof(slBuffCpy);

    slLook->isVisible = 0;
    slLook->name = "Slash";

    // Create the heal card
    Entity hlCard = CreateEntity();
    Item* hlItemCard = AddComponent(hlCard,itemID);
    Lookable* hlLook = AddComponent(hlCard,lookID);
    Card* hlCardCard = AddComponent(hlCard,cardID);

    int hlBuffCpy[] = {4,targetEnt,0,casterEnt,0,heal,30,manaCost,20};// 
    hlItemCard->owner = *ENTFROMCOMP(oid);
    hlCardCard->use = Card_Heal;

    hlCardCard->propBuff = malloc(sizeof(hlBuffCpy));
    memcpy(hlCardCard->propBuff,hlBuffCpy,sizeof(hlBuffCpy));
    hlCardCard->buffSize = sizeof(hlBuffCpy);

    hlLook->isVisible = 0;
    hlLook->name = "Heal";


    //Add the target and caster mods
    oid->cardTarget = AddMod(oid,Mod_Target,0);
    oid->cardCaster = AddMod(oid,Mod_Caster,*ENTFROMCOMP(oid));

    //Create a Deck
    oid->deck = CreateEntity();
    Deck* deck = AddComponent(oid->deck,deckID);
    deck->owner = *ENTFROMCOMP(oid);
    DeckAdd(deck,slCard);
    DeckAdd(deck,hlCard);
    DrawCard(deck);
}
void HumanoidDestroy(void* humanVoid){
    Humanoid* human = humanVoid;
    if(!human){
        log_fatal("Fatal humanoid destroy");
        return;
    }
    for(int i =0;i < 2;i++){
        Item* item = GetComponent(human->hands[i],itemID);
        if(item){
            item->owner = 0;
            Lookable* itemLook = GetComponent(human->hands[i],lookID);
            itemLook->isVisible = 1;
        }
    }
}
void PickUp(Entity picker,int hand,Entity pickee){
    if(hand < 0 || hand > 1){
        log_error("Failed to pick up item with an invalid hand {%d}",hand);
        return;
    }
    Item* item = GetComponent(pickee,itemID);
    Humanoid* human = GetComponent(picker,humanID);
    Connection* conn = GetComponent(picker,connID);
    if(item && human){
        if(item->owner != 0){
            if(conn){
                Sendf(conn,usr_err,"That item is already owned");
            }
            return;
        }
        //pick up the item
        if(human->hands[hand] != 0){
            if(conn){
                Sendf(conn,usr_err,"That hand is full");
            }
            return;
        }
        
        human->hands[hand] = pickee;
        item->owner = picker;
        //run equip on if exists
        if(item->onEquip)
            item->onEquip(item);
        Lookable* look = GetComponent(pickee,lookID);
        if(look)
            look->isVisible = 0;
        if(conn){
            //Lookable* lookee = GetComponent(pickee,lookID);
            char* name = "something?";
            if(look)  name = look->name;
            Sendf(conn,msg,"You picked up %s!",name);
        }
    }else{
        log_error("Either the humanoid {E: %d - V: %d} or the item {E: %d - V: %d} doesn't have their respective "
                  "components (humanoid {%d}, item {%d}",ID(picker),VERSION(picker),ID(pickee),VERSION(pickee),
                  humanID,itemID);
    }
}

void DropItem(Entity dropper,int hand){
    log_debug("performing DropItem");
    if(hand < 0 || hand > 1){
        log_error("Failed to drop item with invalid hand {%d}",hand);
        return;
    }
    Humanoid* human = GetComponent(dropper,humanID);
    if(!human){
        log_error("Failed to drop item from invalid entity {E: %d - V: %d}",ID(dropper),VERSION(dropper));
        return;
    }
    Item* item = GetComponent(human->hands[hand],itemID);
    Connection* conn = GetComponent(dropper,connID);
    if(!item){
        log_error("Failed to drop item in empty hand {%d}",hand);
        return;
    }
    if(item->onDequip)
        item->onDequip(item);
    item->owner = 0;
    Lookable* look = GetComponent(human->hands[hand],lookID);
    if(look)
        look->isVisible = 1;
    human->hands[hand] = 0;
    if(conn)
        Sendf(conn,msg,"You dropped an item");
}
void Attack(Entity attacker,int hand,Entity defender){
    //attacker is attacking defender with their {hand} hand!
    if(hand < 0 || hand > 1){
        log_error("hand index out of range {%d}!",hand);
        return;
    }
    //Get Humanoid Component
    Humanoid* attackHuman = GetComponent(attacker,humanID);
    Lookable* attackerLook = GetComponent(attacker,lookID);
    Humanoid* defenderHuman = GetComponent(defender,humanID);
    Lookable* defenderLook = GetComponent(defender,lookID);
    if(attackHuman != NULL && defenderHuman != NULL && defenderLook && attackerLook){
        MeatBag* defenderMeat = GetComponent(defender,meatID);
        if(defenderMeat != NULL){
            Item* wackItem = GetComponent(attackHuman->hands[hand],itemID);
            if(wackItem != 0){
                //there is an item in hand! Wack time
                //MeatBag* defenderMeat = GetComponent(meatID,defender);
                DealDamage(defender,wackItem->damage);
                TellEveryone(msg,"%s delt %d to %s and now %s has %d health!",attackerLook->name,wackItem->damage,
                             defenderLook->name,defenderLook->name,defenderMeat->health);
            }else{
                //printf("Item doesn't have the Item component!\n");
                //this guy is punching the other guy
                int punchDamage = 5;
                DealDamage(defender,punchDamage);
                TellEveryone(msg,"%s punched %s for %d and now %s has %d health!",attackerLook->name,
                             defenderLook->name,punchDamage,defenderLook->name,defenderMeat->health);
            }
            if(defenderMeat->health <= 0){
                TellEveryone(msg,"Knockout!");
                //DestroyEntity(defender);
                AddComponent(defender,deleteID);//You must(!) call the Delete component's delete system to actually
                // remove it!
            }
            log_info("Performed 'Attack'");
        }else{

            log_error("defender {E: %d - V: %d} doesn't have a meatbag component {%d}!",ID(defender),VERSION
                    (defender),meatID);
            return;
        }
    }else{
        log_error("Either attacker {E: %d - V: %d} or defender {E: %d - V: %d} doesn't have a Humanoid component "
                  "{%d}!",ID(attacker),VERSION(attacker),ID(defender),VERSION(defender),humanID);
    }
}
void AttackString(Entity attacker,List tokens){
    //find the entity that cooresponds
    For_System(humanID,humanoidIter){
        //Humanoid* human = SysIterVal(humanoidIter,Humanoid);
        Lookable* look = GetComponent(humanoidIter.ent,lookID);
        if(look == NULL) continue;
        //search the name
        //create copy of name string
        char* delimitedName = malloc(strlen(look->name)+1);
        strcpy(delimitedName,look->name);
        delimitedName[strlen(look->name)] = 0;
        List nameList = Listify(delimitedName);
        //Lower the name
        LowerStringList(nameList);
        Iter nameIter = MakeIter(&nameList);
        Iter tokenIter = MakeIter(&tokens);
        Inc(&tokenIter);//skip the action token
        while(1){
            int tok = ListCheck(tokenIter);
            int nam = ListCheck(nameIter);
            if(nam == 0) {
                Attack(attacker,0,humanoidIter.ent);
                break;//matched all name tokens
            }
            // }
            if(tok == 0) break;//didn't match all name tokens
            //check if they are the same
            if(strcmp(nameIter.this->data,tokenIter.this->data)!=0){
                //these are not the same
                break;
            }
            Inc(&tokenIter);
            Inc(&nameIter);
        }
        //foundEntity = humanoidIter.ent;
        free(delimitedName);
    }
}
