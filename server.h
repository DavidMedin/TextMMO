#pragma once
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <nng/nng.h>
#include <nng/supplemental/util/platform.h>

#include <ecs.h>
#include <list.h>

nng_stream_listener* listener;
nng_aio* listenIO;//listen  for incoming connections

typedef struct{
    nng_aio* input;
    nng_aio* output;
    nng_stream* stream;
    char* receiveBuff;
    char* sendBuff;
    unsigned int sendBuffEnd;//index to the end
    List actions;//list of Messages (v) to process on main thread. NEW MEMORY
}Connection;
void ConnectionInit(void* emptyConn);
void FreeConnection(Connection* conn);
void DestroyConnection(void* conn);//literally the same as FreeConnection
                                    //but it makes ecs happy
//extern List conns;//list of Connections -> replaced by ecs

nng_mtx* mut;

int ServerInit();
void ServerEnd();
int Sendf(Connection* conn,const char* format,...);//writes to output, and sends
int Sendfa(Connection* conn,const char* format,va_list args);//writes to output, and sends
int Send(Connection* conn);//sends
void WriteOutput(Connection* conn,const char* format,...);//only writes to output
int ReceiveListen(Connection* conn);

/*
//thread functions
void DeferDestruction(Entity ent);//wait for thread(found in Connection component) to finish, then destroies entity
// via DestroyWainting
void DestroyWaiting();//destroys entites waiting to be destroyed

 */