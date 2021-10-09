#include "server.h"
#include "source.h"
extern void DoAction(char* line);
//List conns = {0}; replaced by ecs
void Fatal(const char* func,int error){
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
    nng_mtx_lock(mut);
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
        nng_mtx_unlock(mut);
        return;
    }
    nng_mtx_unlock(mut);
    Sendf(conn,"Welcome!");
    Sendf(conn,"type 'help' for help, I guess.\n");
    ReceiveListen(conn);
    nng_stream_listener_accept(listener,listenIO);
}
void SendCallback(void* voidConn){
    printf("Sent Item\n");
}
void ReceiveCallBack(void* voidConn){
    Connection* conn = voidConn;
    //free what needs to be freed

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
    int read = nng_aio_count(conn->input);
    conn->receiveBuff[read] = 0;
    printf("received data -> '%s'\n",conn->receiveBuff);
    if(strcmp(conn->receiveBuff,"quit")==0) {
        //quiting
        Sendf(conn, "Imagine quitting, I can't");
    }
    //not quitting
    nng_mtx_lock(mut);
    //push to the front of its list
    char* newStr = malloc(read+1);
    memcpy(newStr,conn->receiveBuff,read+1);//include the \0
    printf("%s\n",newStr);
    AddNode(&conn->actions,0,newStr,read);//doesn't include \0
    nng_mtx_unlock(mut);
    ReceiveListen(conn);
        //IFFED

        //nng_aio_wait(conn->output);
        //free Connection

        //kill entity that holds the component
        //Entity ent = *(Entity*)((char*)conn - sizeof(Entity));//super unsafe
        //DeferDestruction(ent);
        //DestroyEntity(ent);//should implicitly destroy the Connection
        //also kills the entity

        //Iter iter = List_FindPointer(&conns,conn);
        //RemoveElementNF(&iter);
        //FreeConnection(conn);
}

nng_mtx* deferedMut;
int ServerInit(){
    int rv;
    if((rv = nng_mtx_alloc(&mut))!=0){
        Fatal("nng_mtx_alloc",rv);
        return 1;
    }
    if((rv=nng_mtx_alloc(&deferedMut))!=0){
        Fatal("nng_mtx_alloc",rv);
        return 1;
    }
    //tcp://138.247.108.215:8080
    if((rv= nng_stream_listener_alloc(&listener,"tcp://138.247.98.67:8080"))!=0){
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
    //nng_aio_stop(conn->input);
    //nng_aio_stop(conn->output);
    nng_aio_free(conn->input);
    nng_aio_free(conn->output);
    nng_stream_free(conn->stream);
    free(conn->receiveBuff);
    free(conn->sendBuff);
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
    //For_Each(conns,connIter) {
    For_System(connID,conIter){
        Connection* conn = conIter.ptr;
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
//void connectionCancel(nng_aio* aio,void* arg,int err){
//    if(err == NNG_ECANCELED){
//        printf("canceled\n");
//    }
//    nng_aio_finish(aio,err);
//}
void ConnectionInit(void* emptyConn){
    Connection* conn = emptyConn;
    int rv;
    if((rv=nng_aio_alloc(&conn->output,SendCallback,conn))!=0){
        Fatal("output nng_aio_alloc",rv);
        return;
    }
    //nng_aio_defer(conn->output,connectionCancel,NULL);
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
    conn->actions.count=0;
    conn->actions.start=NULL;
    conn->actions.end=NULL;
}

List deferedEntities = {0};
void DeferDestruction(Entity ent){
    Entity* data = malloc(sizeof(Entity));
    *data = ent;
    nng_mtx_lock(deferedMut);
    PushBack(&deferedEntities,data,sizeof(Entity));
    nng_mtx_unlock(deferedMut);
    //nng_aio_cancel(GetComponent(connID,ent));//maybe
}
void DestroyWaiting(){
    nng_mtx_lock(deferedMut);
    For_Each(deferedEntities,deferedIter){
        DestroyEntity(*Iter_Val(deferedIter,Entity));
        RemoveElement(&deferedIter);
    }
    nng_mtx_unlock(deferedMut);
}
