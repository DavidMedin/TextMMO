#pragma once
#include "ecs.h"
#include "source.h"
//
// Created by DSU on 10/19/2021.
//

#include "lookable.h"
void Look(Entity looker){
    Connection* conn = GetComponent(looker,connID);
    if(conn == NULL){
        log_error("This entity {E: %d - V: %d} doesn't have the 'Connection' {%d} component!",ID(looker),VERSION
                (looker),connID);
        return;
    }
    //print all entities that isn't you
    WriteOutput(conn,"You look around and see:");
    int found = 0;
    For_System(lookID, lookIter) {
        Lookable *look = SysIterVal(lookIter, Lookable);
        if (look->isVisible==1 && lookIter.ent != looker) {
            //this isn't us
            int testItem = HasComponent(lookIter.ent,itemID);
            char* prepend = testItem ? "<color=blue>" : "";
            char* append = testItem ? "</color>" : "";
            WriteOutput(conn,"\n\t* %s%s%s", prepend,look->name,append);
            found = 1;
        }
    }
    if (found == 0) {
        WriteOutput(conn,"nothing");
    } else{}
    Send(conn);
    log_info("Performed 'Look'");
}
