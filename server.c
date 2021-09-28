#include "server.h"
void Fatal(char* func,int error){
    printf("%s error: %s\n",func,nng_strerror(error));
}
int StartStuff(){
    //int openError = nng_pair0_open(&sock);
    //if(openError != 0){
    //    Fatal("nng_pair0_open",openError);
    //    return 1;
    //}
    printf("yo\n");
    int listenError = nng_listen(sock,"tcp://localhost:8081",NULL,0);
    if(listenError != 0){
        Fatal("nng_listen",listenError);
        return 1;
    }
    ////set timeout
    //int timeError = nng_setopt_ms(sock,NNG_OPT_RECVTIMEO,100);
    //if(timeError != 0){
    //    Fatal("nng_setopt_ms (recvtimeo)",timeError);
    //    return 1;
    //}
    //timeError = nng_setopt_ms(sock,NNG_OPT_SENDTIMEO,100);
    //if(timeError != 0){
    //    Fatal("nng_setopt_ms (sendtimeo)",timeError);
    //    return 1;
    //}
    return 0;
}

int SendStuff(const char* data){
    int sendError = nng_send(sock,(void*)data,strlen(data)+1,0);
    if(sendError != 0){
        Fatal("nng_send",sendError);
        return 1;
    }
    return 0;
}
int RecieveStuff(char** newData,size_t* size){
    int receiveError = nng_recv(sock,(void*)newData,size,NNG_FLAG_ALLOC);
    if(receiveError != 0){
        Fatal("nng_recv",receiveError);
    }
    return 0;
}