#include "GameActions.h"
#include "source.h"
void Look(Entity looker){
    //print all entities that isn't you
    WriteOutput("You look around and see");
    int found = 0;
    For_System(humanID, humanoidIter) {
        Humanoid *humanComp = SysIterVal(humanoidIter, Humanoid);
        if (humanoidIter.ent != looker) {
            //this isn't us
            if(humanoidIter.i != 0)
                WriteOutput(",");
            WriteOutput(" %s", humanComp->name);
            found = 1;
        }
    }
    if (found == 0) {
        WriteOutput("nobody");
    } else{}
    //WriteOutput("");
    WriteOutput(".\n");
    Send();
}
void SpawnGoblin(){
    //spawn a goblin
    Sendf("a wild goblin appears!");
    Entity goblin = CreateEntity();
    AddComponent(goblin, humanID);
    ((Humanoid *) GetComponent(humanID, goblin))->name = "the goblin";
    AddComponent(goblin, meatID);
    AddComponent(goblin,aiID);
}
void Attack(Entity attacker,int hand,Entity defender){
    //attacker is attacking defender with their {hand} hand!
    if(hand < 0 || hand > 1){
        printf("hand index out of range! %d \n",hand);
        return;
    }
    //Get Humanoid Component
    Humanoid* attackHuman = GetComponent(humanID,attacker);
    Humanoid* defenderHuman = GetComponent(humanID,defender);
    if(attackHuman != NULL && defenderHuman != NULL){
        MeatBag* defenderMeat = GetComponent(meatID,defender);
        if(defenderMeat != NULL){
            Item* wackItem = GetComponent(itemID,attackHuman->hands[hand]);
            if(wackItem != 0){
                //there is an item in hand! Wack time
                //MeatBag* defenderMeat = GetComponent(meatID,defender);
                DealDamage(defender,wackItem->damage);
                Sendf("%s delt %d to %s and now %s has %d health!\n",attackHuman->name,wackItem->damage,
                      defenderHuman->name,defenderHuman->name,defenderMeat->health);
            }else{
                //printf("Item doesn't have the Item component!\n");
                //this guy is punching the other guy
                DealDamage(defender,5);
                Sendf("%s punched %s for %d and now %s has %d health!\n",attackHuman->name,
                      defenderHuman->name,5,defenderHuman->name,defenderMeat->health);
            }
            if(defenderMeat->health <= 0){
                Sendf("Knockout!\n");
                DestroyEntity(defender);
            }
        }else{

            printf("defender doesn't have a meatbag component!\n");
            return;
        }
    }else{
        printf("Either attacker or defender doesn't have a Humanoid component!\n");
    }
}
void AttackString(Entity attacker,List tokens){
    //find the entity that cooresponds
    For_System(humanID,humanoidIter){
        Humanoid* human = SysIterVal(humanoidIter,Humanoid);
        //search the name
        //create copy of name string
        char* delimitedName = malloc(strlen(human->name)+1);
        strcpy(delimitedName,human->name);
        delimitedName[strlen(human->name)] = 0;
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
