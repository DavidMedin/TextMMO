#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <nng/nng.h>

nng_stream_listener* listener;
nng_stream* out;
nng_stream* in;
nng_aio* output;
nng_aio* input;
nng_aio* listenIO;
char* receiveData;
char* sendBuff;
unsigned int sendBuffEnd;//index to the end

int ServerInit();
int Sendf(const char* format,...);//writes to output, and sends
int Send();//sends
void WriteOutput(const char* format,...);//only writes to output
int ReceiveListen();
