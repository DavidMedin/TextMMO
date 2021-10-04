#include "server.h"
extern void DoAction(char* line);
List conns = {0};
void Fatal(char* func,int error){
    printf("%s error: %s\n",func,nng_strerror(error));

}

void Listen(void* nothing){

    int rv;
    if((rv=nng_aio_result(listenIO))!=0) {
        Fatal("Listen nng_aio_result", rv);
        return;
    }
    printf("Found a connection\n");
    stream = nng_aio_get_output(listenIO,0);
    if(stream == NULL){
        printf("couldn't get output\n");
        return;
    }

    nng_iov vec;
    sendBuff = malloc(256);
    vec.iov_buf = sendBuff;
    vec.iov_len = 256;
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
    Sendf("Welcome!");
    Sendf("type 'help' for help, I guess.\n");
    ReceiveListen();
}
void SendCallback(void* nothing){
    printf("Sent Item\n");
}
extern char* receive;
void ReceiveCallBack(void* nothing){
    int rv;
    if((rv=nng_aio_result(input))!=0){
        if(rv == 7){//7 is object closed, probably freeing everything
            return;
        }
        Fatal("Receive nng_aio_result",rv);
        printf("%d\n",rv);
        nng_stream_listener_accept(listener,listenIO);
        return;
    }
    printf("received something\n") ;
    receiveData[nng_aio_count(input)] = 0;
    printf("%s\n",receiveData);
    nng_mtx_lock(mut);
    receive = receiveData;
    nng_mtx_unlock(mut);
    if(strcmp(receive,"quit")!=0){
        ReceiveListen();//not quitting
    }else{
        //quiting

    }
}

int ServerInit(){
    sendBuffEnd = 0;
    int rv;
    if((rv = nng_mtx_alloc(&mut))!=0){
        Fatal("nng_mtx_alloc",rv);
        return 1;
    }
    if((rv= nng_stream_listener_alloc(&listener,"tcp://127.0.0.1:8080"))!=0){
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
    //if((rv=nng_aio_alloc(&output,SendCallback,NULL))!=0){
    //    Fatal("output nng_aio_alloc",rv);
    //    return 1;
    //}
    //if((rv=nng_aio_alloc(&input,ReceiveCallBack,NULL))!=0){
    //    Fatal("input nng_aio_alloc",rv);
    //    return 1;
    //}
    ;
    nng_stream_listener_accept(listener,listenIO);
    return 0;
}
void ServerEnd(){
    printf("freeing\n");
    nng_stream_listener_close(listener);
    nng_stream_listener_free(listener);
    nng_aio_wait(input);
    nng_aio_free(input);
    nng_aio_free(output);
    nng_aio_free(listenIO);
    nng_stream_close(stream);
    nng_stream_free(stream);
    nng_mtx_free(mut);
}

int Send(){
    sendBuffEnd = 0;
    nng_aio_wait(output);
    nng_stream_send(stream,output);
}
int Sendf(const char* format,...){
    va_list args;
    va_start(args,format);
    vsprintf(sendBuff,format,args);
    Send();
    return 0;
}
void WriteOutput(const char* format,...){
    va_list args;
    va_start(args,format);
    sendBuffEnd += vsprintf(sendBuff+sendBuffEnd,format,args);
}
int ReceiveListen(){
    nng_stream_recv(stream,input);
    return 0;
}