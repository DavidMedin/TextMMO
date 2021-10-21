//
// Created by DSU on 10/19/2021.
//

#include "humanoid.h"
#include "log.h"
//#include "GameActions.h"

int UpdateHumanoid(Entity ent,char* line){
    List tokens = Listify(line);
    if(tokens.count != 0) {
        char *action = tokens.start->data;
        if (strcmp(action, "attack") == 0) {
            AttackString(ent, tokens);
            return 0;
        } else if (strcmp(action, "look") == 0) {
            Look(ent);
            return 0;
        }else if(strcmp(action, "help") == 0){
            //print some helpful stuff
            Sendf(GetComponent(ent,connID),"Commands----------\n\t*look\t--look around\n\t*attack {enemy "
                                           "name}\t--attack that "
                                           "bitch\n\t*pick up {item}\t--pick up the item\n\t*help\t--you are "
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
        }else if(strcmp(action,"drop")==0){
            DropItem(ent,0);
            return 0;
        }
    }
    //CallSystem(AIUpdate,humanID,aiID);
    return 1;
}
extern int quitting;
void HumanConnUpdate(Entity ent) {//System for Humanoid and Connection
    Connection *conn = GetComponent(ent, connID);
    if (conn->actions.count > 0) {
        //do actions
        For_Each(conn->actions, actionIter) {
            char *msg = Iter_Val(actionIter, char);
            char *old = malloc(strlen(msg) + 1);//preserve message before strtok
            strcpy(old, msg);
            old[strlen(msg)] = 0;
            if (strcmp(msg, "quit") == 0) {
                AddComponent(ent, deleteID);
                free(old);
                break;
            } else if (UpdateHumanoid(ent, msg)) {
                TellEveryone("%s", old);
            }
            free(old);
            RemoveElement(&actionIter);
        }
    }
}
void HumanAIUpdate(Entity ent);//System for Humanoid and AI
void HumanoidInit(void* human){
    Humanoid* oid = human;
    oid->hands[0] = 0;//empty left hand
    oid->hands[1] = 0;//empty right hand
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
                Sendf(conn,"<color=red>That item is already owned</color>");
            }
            return;
        }
        //pick up the item
        if(human->hands[hand] != 0){
            if(conn){
                Sendf(conn,"<color=red>That hand is full</color>");
            }
            return;
        }
        human->hands[hand] = pickee;
        item->owner = picker;
        Lookable* look = GetComponent(pickee,lookID);
        if(look)
            look->isVisible = 0;
        if(conn){
            Lookable* lookee = GetComponent(pickee,lookID);
            char* name = "something?";
            if(lookee)  name = lookee->name;
            Sendf(conn,"You picked up %s!",name);
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
    item->owner = 0;
    Lookable* look = GetComponent(human->hands[hand],lookID);
    if(look)
        look->isVisible = 1;
    human->hands[hand] = 0;
    if(conn)
        Sendf(conn,"You dropped an item");
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
                TellEveryone("%s delt %d to %s and now %s has %d health!",attackerLook->name,wackItem->damage,
                             defenderLook->name,defenderLook->name,defenderMeat->health);
            }else{
                //printf("Item doesn't have the Item component!\n");
                //this guy is punching the other guy
                int punchDamage = 5;
                DealDamage(defender,punchDamage);
                TellEveryone("%s punched %s for %d and now %s has %d health!",attackerLook->name,
                             defenderLook->name,punchDamage,defenderLook->name,defenderMeat->health);
            }
            if(defenderMeat->health <= 0){
                TellEveryone("Knockout!");
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
        Humanoid* human = SysIterVal(humanoidIter,Humanoid);
        Lookable* look = GetComponent(humanoidIter.ent,lookID);
        if(look == NULL) continue;
        //search the name
        //create copy of name string
        char* delimitedName = malloc(strlen(look->name)+1);
        strcpy(delimitedName,look->name);
        delimitedName[strlen(look->name)] = 0;
        List nameList = Listify(delimitedName);
        Iter nameIter = MakeIter(&nameList);
        Iter tokenIter = MakeIter(&tokens);
        Inc(&tokenIter);//skip the action token
        while(1){
            int tok = Inc(&tokenIter);
            int nam = Inc(&nameIter);
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
        }
        //foundEntity = humanoidIter.ent;
        free(delimitedName);
    }
}
