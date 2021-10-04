#include "server.h"
#include "source.h"
extern void DoAction(char* line);
//List conns = {0}; replaced by ecs
void Fatal(char* func,int error){
    printf("%s error: %s\n",func,nng_strerror(error));

}
char* names[] = {"Jimmy","Garret","Karina"};
int nameCount = 3;
void SendCallback(void* nothing);
void ReceiveCallBack(void* nothing);
void Listen(void* nothing){
    //create a new entity, and populate it with the right Components
    int rv;
    if((rv=nng_aio_result(listenIO))!=0) {
        Fatal("Listen nng_aio_result", rv);
        return;
    }
    printf("Found a connection\n");
    Entity newPlayer = CreateEntity();
    AddComponent(newPlayer,humanID);
    static int next = 0;
    ((Humanoid*) GetComponent(humanID,newPlayer))->name = names[next];
    if(++next >= nameCount){
        next = 0;
    }
    AddComponent(newPlayer,meatID);
    AddComponent(newPlayer,connID);
    Connection* conn = GetComponent(connID,newPlayer);
    conn->stream = nng_aio_get_output(listenIO,0);
    if(conn->stream == NULL){
        printf("couldn't get output\n");
        return;
    }
    Sendf(conn,"Welcome!");
    Sendf(conn,"type 'help' for help, I guess.\n");
    ReceiveListen(conn);
}
void SendCallback(void* voidConn){
    printf("Sent Item\n");
}
extern char* receive;
void ReceiveCallBack(void* voidConn){
    Connection* conn = voidConn;
    int rv;
    if((rv=nng_aio_result(conn->input))!=0){
        if(rv == 7){//7 is object closed, probably freeing everything
            return;
        }
        Fatal("Receive nng_aio_result",rv);
        printf("%d\n",rv);
        nng_stream_listener_accept(listener,listenIO);
        return;
    }
    conn->receiveBuff[nng_aio_count(conn->input)] = 0;
    printf("received data -> '%s'\n",conn->receiveBuff);
    if(strcmp(receive,"quit")!=0){
        //not quitting
        nng_mtx_lock(mut);
        receive = conn->receiveBuff;
        nng_mtx_unlock(mut);
        ReceiveListen(conn);
    }else{
        //quiting
        Sendf(conn,"Imagine quitting, I can't");
        nng_aio_wait(conn->output);
        //free Connection
        Iter iter = List_FindPointer(&conns,conn);
        RemoveElementNF(&iter);
        FreeConnection(conn);
    }
}

int ServerInit(){
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
    nng_stream_listener_accept(listener,listenIO);
    return 0;
}
void FreeConnection(Connection* conn){
    printf("destroying connection\n");
    nng_aio_wait(conn->input);
    nng_aio_free(conn->input);
    nng_aio_free(conn->output);
    nng_stream_close(conn->stream);
    nng_stream_free(conn->stream);
}
void DestroyConnection(void* conn){
    FreeConnection(conn);
}
void ServerEnd(){
    printf("freeing\n");
    nng_stream_listener_close(listener);
    nng_stream_listener_free(listener);
    nng_aio_free(listenIO);
    nng_mtx_free(mut);
    For_Each(conns,connIter) {
        Connection* conn = Iter_Val(connIter,Connection);
        FreeConnection(conn);
    }

}

int Send(Connection* conn){
    conn->sendBuffEnd = 0;
    nng_aio_wait(conn->output);
    nng_stream_send(conn->stream,conn->output);
}
int Sendf(Connection* conn,const char* format,...){
    va_list args;
    va_start(args,format);
    vsprintf(conn->sendBuff,format,args);
    Send(conn);
    return 0;
}
int Sendfa(Connection* conn,const char* format,va_list args){
    vsprintf(conn->sendBuff,format,args);
    Send(conn);
    return 0;
}
void WriteOutput(Connection* conn,const char* format,...){
    va_list args;
    va_start(args,format);
    conn->sendBuffEnd += vsprintf(conn->sendBuff+conn->sendBuffEnd,format,args);
}
int ReceiveListen(Connection* conn){
    nng_stream_recv(conn->stream,conn->input);
    return 0;
}
void ConnectionInit(void* emptyConn){
    Connection* conn = emptyConn;
    int rv;
    if((rv=nng_aio_alloc(&conn->output,SendCallback,conn))!=0){
        Fatal("output nng_aio_alloc",rv);
        return;
    }
    if((rv=nng_aio_alloc(&conn->input,ReceiveCallBack,conn))!=0){
        Fatal("input nng_aio_alloc",rv);
        return;
    }
    nng_iov vec;
    conn->sendBuff = malloc(256);
    vec.iov_buf = conn->sendBuff;
    vec.iov_len = 256;
    if((rv=nng_aio_set_iov(conn->output,1,&vec))!=0){
        Fatal("output nng_aio_set_iov",rv);
        return;
    }
    conn->receiveBuff = malloc(256);
    vec.iov_buf = conn->receiveBuff;
    vec.iov_len = 256;
    if((rv=nng_aio_set_iov(conn->input,1,&vec))!=0){
        Fatal("input nng_aio_set_iov",rv);
        return;
    }
}
