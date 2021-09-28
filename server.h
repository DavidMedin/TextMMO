#include <stdio.h>
#include <string.h>
#include <nng/nng.h>

nng_stream_listener* listener;
nng_stream* out;
nng_aio* io;

int StartStuff();
int SendStuff();
int RecieveStuff();
