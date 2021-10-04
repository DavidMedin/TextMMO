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
}Connection;
extern List conns;//list of Connections

nng_mtx* mut;
char* receiveData;
char* sendBuff;
unsigned int sendBuffEnd;//index to the end

int ServerInit();
void ServerEnd();
int Sendf(const char* format,...);//writes to output, and sends
int Send();//sends
void WriteOutput(const char* format,...);//only writes to output
int ReceiveListen();
