#include "server.h"
void Fatal(char* func,int error){
    printf("%s error: %s\n",func,nng_strerror(error));

}

void Listen(void* nothing){
    printf("Found a connection\n");

    printf("Something to do with aio just happened\n");
    int rv;
    out = nng_aio_get_output(listenIO,0);
    in = out;
    if(out == NULL){
        printf("couldn't get output\n");
        return;
    }

    nng_iov vec;
    vec.iov_buf = "this is text";
    vec.iov_len = strlen(vec.iov_buf);
    if((rv=nng_aio_set_iov(output,1,&vec))!=0){
        Fatal("output nng_aio_set_iov",rv);
        return;
    }
    receiveData = malloc(256);
    vec.iov_buf = receiveData;
    vec.iov_len = 256;
    if((rv=nng_aio_set_iov(input,1,&vec))!=0){
        Fatal("input nng_aio_set_iov",rv);
        return;
    }

    SendStuff();
    RecieveStuff();
}
void Send(void* nothing){
    printf("Sent Item\n");
}
void Receive(void* nothing){
    printf("received something\n") ;
    receiveData[nng_aio_count(input)] = 0;
    printf("%s\n",receiveData);
    RecieveStuff();
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

    if((rv=nng_aio_alloc(&listenIO,Listen,NULL))!=0){
        Fatal("listenIO nng_aio_alloc",rv);
        return 1;
    }
    if((rv=nng_aio_alloc(&output,Send,NULL))!=0){
        Fatal("output nng_aio_alloc",rv);
        return 1;
    }
    if((rv=nng_aio_alloc(&input,Receive,NULL))!=0){
        Fatal("input nng_aio_alloc",rv);
        return 1;
    }
    nng_stream_listener_accept(listener,listenIO);
    return 0;
}

int SendStuff(){
    nng_stream_send(out,output);
    return 0;
}
int RecieveStuff(){
    nng_stream_recv(in,input);
    return 0;
}