#include "GameActions.h"
#include "source.h"
void TellEveryone(const char* format,...){
    va_list args;
    va_start(args,format);
    For_System(connID,connIter){
        Sendfa(connIter.ptr,format,args);
    }
}
void SpawnGoblin(){
    //spawn a goblin
    TellEveryone("A wild goblin appears!");
    Entity goblin = CreateEntity();
    AddComponent(goblin, humanID);
    AddComponent(goblin,lookID);
    ((Lookable *) GetComponent( goblin,lookID))->name = "the goblin";
    AddComponent(goblin, meatID);
    AddComponent(goblin,aiID);
    log_info("Performed 'SpawnGoblin'");
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
