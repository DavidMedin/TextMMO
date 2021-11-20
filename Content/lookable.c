#pragma once
#include "ecs.h"
#include "source.h"
#include "lookable.h"
void Look(Entity looker){
    Humanoid* human = GetComponent(looker,humanID);
    Connection* conn = GetComponent(looker,connID);
    if(conn == NULL){
        log_error("This entity {E: %d - V: %d} doesn't have the 'Connection' {%d} component!",ID(looker),VERSION
                (looker),connID);
        return;
    }
    if(!human){
        log_error("A non-human {E: %d - V: %d} tried to look!",ID(looker),VERSION(looker));
        return;
    }
    //print all entities that isn't you
    WriteOutput(conn,"%c",msg);
    WriteOutput(conn,"You look around and see:");
    //clear the humanoid's veiw
    FreeList(&human->view);// will free the baskets too
    int found = 0;
    int index = 0;
    For_System(lookID, lookIter) {
        Lookable *look = SysIterVal(lookIter, Lookable);
        if (look->isVisible==1 && lookIter.ent != looker) {
            //this isn't us.
            //Add to humanoid's view
            log_debug("adding to view index %d : {E: %d - V: %d}",index,ID(lookIter.ent),VERSION(lookIter.ent));
            //AddNode(&human->view,0,CreateBasket(sizeof(Entity),&lookIter.ent),sizeof(void*));
            PushBack(&human->view,CreateBasket(sizeof(Entity),&lookIter.ent),sizeof(void*));
            int testItem = HasComponent(lookIter.ent,itemID);
            char* prepend = testItem ? "<color=blue>" : "";
            char* append = testItem ? "</color>" : "";
            WriteOutput(conn,"\n\t[%d] %s%s%s",index++,prepend,look->name,append);
            found = 1;
        }
    }
    if (found == 0) {
        WriteOutput(conn,"nothing");
    } else{}
    Send(conn);
    log_info("Performed 'Look'");
}
extern void LowerString(char* str);
Entity Look_StringToEntity(char* string){
    For_System(lookID,lookIter){
        Lookable* look = SysIterVal(lookIter,Lookable);
        char underName[strlen(look->name)+1];
        underName[strlen(look->name)] = 0;
        strcpy(underName,look->name);
        LowerString(underName);
        if(strcmp(string,underName)==0){
            return lookIter.ent;
        }
    }
    return 0;
}
 void LookableInit(void* lookable){
    Lookable* look = lookable;
    look->name = "Unknown Thing";
    look->isVisible = 1;
}
