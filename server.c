#include "server.h"
void Fatal(char* func,int error){
    printf("%s error: %s\n",func,nng_strerror(error));

}

void thing(void* nothing){
    printf("Something to do with aio just happend\n");
    int rv;
    out = nng_aio_get_output(io,0);
    if(out == NULL){
        printf("couldn't get output\n");
        return;
    }

    nng_iov vec;
    vec.iov_buf = "this is text";
    vec.iov_len = strlen(vec.iov_buf);
    if((rv=nng_aio_set_iov(io,1,&vec))!=0){
        Fatal("nng_aio_set_iov",rv);
        return;
    }
    SendStuff();
}
int StartStuff(){
    int rv;
    if((rv= nng_stream_listener_alloc(&listener,"tcp://localhost:8081"))!=0){
        Fatal("nng_stream_listener_alloc",rv);
        return 1;
    }
    printf("allocated listener\n");
    if((rv=nng_stream_listener_listen(listener))!=0){
        Fatal("nng_stream_listener_listen",rv);
        return 1;
    }
    printf("listening with listener\n");

    if((rv=nng_aio_alloc(&io,thing,NULL))!=0){
        Fatal("nng_aio_alloc",rv);
        return 1;
    }
    nng_stream_listener_accept(listener,io);

    //int openError = nng_pair0_open(&sock);
    //if(openError != 0){
    //    Fatal("nng_pair0_open",openError);
    //    return 1;
    //}
    //printf("yo\n");
    //int listenError = nng_listen(sock,"tcp://localhost:8081",NULL,0);
    //if(listenError != 0){
    //    Fatal("nng_listen",listenError);
    //    return 1;
    //}
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

int SendStuff(){
    //int sendError = nng_send(sock,(void*)data,strlen(data)+1,0);
    //if(sendError != 0){
    //    Fatal("nng_send",sendError);
    //    return 1;
    //}
    nng_stream_send(out,io);
    return 0;
}
int RecieveStuff(){
    //int receiveError = nng_recv(sock,(void*)newData,size,NNG_FLAG_ALLOC);
    //if(receiveError != 0){
    //    Fatal("nng_recv",receiveError);
    //}
    nng_stream_recv(out,io);
    return 0;
}