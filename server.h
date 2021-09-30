#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <nng/nng.h>

nng_stream_listener* listener;
nng_stream* out;
nng_stream* in;
nng_aio* output;
nng_aio* input;
nng_aio* listenIO;
char* receiveData;

int StartStuff();
int SendStuff();
int RecieveStuff();
