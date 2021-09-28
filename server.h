#include <stdio.h>
#include <string.h>
#include <nng/nng.h>
nng_socket sock;//a pair socket
nng_stream stream;

int StartStuff();
int SendStuff(const char* data);
int RecieveStuff(char** newData,size_t* size);
