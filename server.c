#include "server.h"
#include "source.h"
extern void DoAction(char* line);
//List conns = {0}; replaced by ecs
void Fatal(const char* func,int error){
    log_error("%s error: %s",func,nng_strerror(error));

}
char* names[] = {"Jimmy","Garret","Karina"};
int nameCount = 3;
//void SendCallback(void* nothing);
//void ReceiveCallBack(void* nothing);
void Listen(void* nothing){
    //create a new entity, and populate it with the right Components
    int rv;
    if((rv=nng_aio_result(listenIO))!=0) {
        Fatal("Listen nng_aio_result", rv);
        return;
    }
    log_info("Found a connection");
    nng_mtx_lock(mut);
    Entity newPlayer = CreateEntity();
    //AddComponent(newPlayer,humanID);
    //AddComponent(newPlayer,lookID);
    //static int next = 0;
    //((Lookable *) GetComponent(newPlayer,lookID))->name = names[next];
    //if(++next >= nameCount){
    //    next = 0;
    //}
    //AddComponent(newPlayer,meatID);
    AddComponent(newPlayer,connID);
    Connection* conn = GetComponent(newPlayer,connID);
    conn->loggingIn = 1;
    conn->stream = nng_aio_get_output(listenIO,0);
    if(conn->stream == NULL){
        log_error("Couldn't get output");
        nng_mtx_unlock(mut);
        return;
    }
    nng_mtx_unlock(mut);
    ReceiveListen(conn);
    nng_stream_listener_accept(listener,listenIO);
}
void SendCallback(void* voidConn){
    int entity = (int)voidConn;
    Connection* conn = GetComponent(entity,connID);
    if(conn==NULL){
        log_error("Failed to send message to old entity {E: %d - V: %d}!",ID(entity),VERSION(entity));
        return;
    }
    log_info("sent message {%d : %s} to entity {E: %d - V: %d}",(char)*conn->sendBuff,conn->sendBuff+1,ID(entity),VERSION(entity));
}

#define ENTFROMCOMP(comp) (int*)(((char*)comp)-sizeof(int))
void ReceiveCallBack(void* ent){
    int entity = (int)ent;
    Connection* conn = GetComponent(entity,connID);
    if(conn==NULL){
        log_fatal("Entity in Receive {E: %d - V: %d} doesn't contain Connection {%d}!!!",ID(entity),VERSION(entity),
                  connID);
        nng_mtx_lock(mut);
        AddComponent(entity, deleteID);
        nng_mtx_unlock(mut);
        return;
    }
    //free what needs to be freed

    int rv;
    if((rv=nng_aio_result(conn->input))!=0){
        if(rv == 7 || rv == 20 || rv == 31) {
            switch (rv) {
                case 7:
                    log_warn("Object is closed");
                    break;
                case 20:
                    log_warn("Operation was cancelled");
                    return;
                case 31:
                    log_warn("Connection was closed");
                    break;
            }
            //test to see if we should delete, because this might be because of the entity destructor.
            if(!IsEntityValid(entity)){
                log_warn("Entity is already dead, skipping deletion");
                return;
            }
        }else{
            Fatal("Receive nng_aio_result",rv);
            log_error("Receive error: %d",rv);
        }
        //    //7 is object close
        //    //20 is cancelled
        //    //31 is connection shutdown
        //    return;
        //}
        nng_mtx_lock(mut);
        AddComponent(*ENTFROMCOMP(conn), deleteID);//done worry, be happy
        nng_mtx_unlock(mut);
        return;
    }
    int read = (int)nng_aio_count(conn->input);
    if(read == 0){
        log_warn("read zero");
        nng_mtx_lock(mut);
        AddComponent(*ENTFROMCOMP(conn), deleteID);//done worry, be happy
        nng_mtx_unlock(mut);
        return;
    }
    if(read >= 256)
        log_warn("Received buffer >= 256! Might cause buffer overflow!");
    conn->receiveBuff[read] = 0;
    log_info("received data -> '%s'",conn->receiveBuff);
    if(!conn->loggingIn && strcmp(conn->receiveBuff,"quit")==0) {
        //quiting
        Sendf(conn,msg,"Imagine quitting, I can't");
    }
    //not quitting
    nng_mtx_lock(mut);
    log_info("Obtained lock");
    //push to the front of its list
    char* newStr = malloc(read+1);
    memcpy(newStr,conn->receiveBuff,read+1);//include the \0
    PushBack(&(conn->actions),newStr,read);//doesn't include \0
    log_debug("Connection action list length: %d -> %s\n",conn->actions.count,conn->actions.count ? conn->actions
    .start->data : "nothing");
    nng_mtx_unlock(mut);
    ReceiveListen(conn);
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
    if((rv= nng_stream_listener_alloc(&listener,"tcp://localhost:8080"))!=0){
        //if((rv= nng_stream_listener_alloc(&listener,"tcp://172.17.207.29:8080"))!=0){
    //if((rv= nng_stream_listener_alloc(&listener,"tcp://138.247.98.67:8080"))!=0){
        Fatal("nng_stream_listener_alloc",rv);
        return 1;
    }
    log_info("allocated listener");
    if((rv=nng_stream_listener_listen(listener))!=0){
        Fatal("nng_stream_listener_listen",rv);
        return 1;
    }
    log_info("listening with listener");

    if((rv=nng_aio_alloc(&listenIO,Listen,NULL))!=0){
        Fatal("listenIO nng_aio_alloc",rv);
        return 1;
    }
    nng_stream_listener_accept(listener,listenIO);
    return 0;
}
void FreeConnection(Connection* conn){
    log_info("destroying connection");
    //tell everyone
    For_System(connID,connIter){
        if(connIter.ent != *ENTFROMCOMP(conn)){
            Sendf(connIter.ptr,msg,"%s disconnected",conn->username);
        }
    }
    nng_aio_stop(conn->input);
    nng_aio_stop(conn->output);
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
    log_info("freeing connection");
    nng_stream_listener_close(listener);
    nng_stream_listener_free(listener);
    nng_aio_free(listenIO);
    nng_mtx_free(mut);
    For_System(connID,conIter){
        Connection* conn = conIter.ptr;
        FreeConnection(conn);
    }

}

int Send(Connection* conn){
    if(conn->loggingIn) return 1;
    conn->sendBuffEnd = 0;
    nng_aio_wait(conn->output);
    nng_stream_send(conn->stream,conn->output);
}
int Sendf(Connection* conn,Header head,const char* format,...){
    va_list args;
    va_start(args,format);
    nng_aio_wait(conn->output);
    *conn->sendBuff = (char)head;
    vsprintf(conn->sendBuff+1,format,args);
    Send(conn);
    return 0;
}
int Sendfa(Connection* conn,Header head,const char* format,va_list args){
    *conn->sendBuff = (char)head,
    vsprintf(&conn->sendBuff[1],format,args);
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
void ConnectionInit(void* emptyConn) {
    Connection *conn = emptyConn;
    int rv;
    if ((rv = nng_aio_alloc(&conn->output, SendCallback,*ENTFROMCOMP(conn))) != 0) {
        Fatal("output nng_aio_alloc", rv);
        return;
    }
    //nng_aio_defer(conn->output,connectionCancel,NULL);
    if ((rv = nng_aio_alloc(&conn->input, ReceiveCallBack,*ENTFROMCOMP(conn))) != 0) {
        Fatal("input nng_aio_alloc", rv);
        return;
    }
    nng_iov vec;
    conn->sendBuff = malloc(256);
    vec.iov_buf = conn->sendBuff;
    vec.iov_len = 256;
    if ((rv = nng_aio_set_iov(conn->output, 1, &vec)) != 0) {
        Fatal("output nng_aio_set_iov", rv);
        return;
    }
    conn->receiveBuff = malloc(256);
    vec.iov_buf = conn->receiveBuff;
    vec.iov_len = 256;
    if ((rv = nng_aio_set_iov(conn->input, 1, &vec)) != 0) {
        Fatal("input nng_aio_set_iov", rv);
        return;
    }
    conn->actions.count = 0;
    conn->actions.start = NULL;
    conn->actions.end = NULL;
}
void TellEveryone(Header head,const char* format,...){
    va_list args;
    va_start(args,format);
    For_System(connID,connIter){
        Sendfa(connIter.ptr,head,format,args);
    }
}
void TryLogin(Entity entity){
    Connection* conn = GetComponent(entity,connID);
    if(conn->loggingIn == 0 && conn->actions.count != 0){
        log_error("Action stack was not destroyed.");
        return;
    }
    if(conn->actions.count == 0) return;
    //it is assumed that any 'action' that comes through here is a name.
    //check if this name is taken
    For_System(connID,connIter){
        Connection* otherConn = connIter.ptr;
        if(connIter.ent == entity || otherConn->loggingIn == 0) break;
        if(otherConn->username == NULL){
            log_fatal("Entity {E: %d - V: %d} is not logging in, and their username",ID(connIter.ent),VERSION(connIter.ent));
            break;
        }
        if(strcmp(otherConn->username,conn->username)!=0){
            //send error to client
            Sendf(conn, usr_err, "That username is taken.");
            //Send(conn);
            return;
        }
    }
    conn->username = conn->actions.start->data;
    Iter tmpIter = List_FindPointer(&conn->actions,conn->username);
    RemoveElementNF(&tmpIter);
    if(conn->actions.count > 0){
        log_warn("Still remaining actions in 'stack'.");
    }
    Entity connEnt = *ENTFROMCOMP(conn);
    AddComponent(connEnt,humanID);
    Lookable* look = AddComponent(connEnt,lookID);
    look->name = conn->username;
    AddComponent(connEnt,meatID);
    conn->loggingIn = 0;
    Sendf(conn,login,"");//tell the client that the login was sucessful
    Sendf(conn,msg,"Welcome, %s! {E: %d - V: %d}",conn->username,ID(entity),VERSION(entity));
    Sendf(conn,msg,"type 'help' for help, I guess.");
    For_System(connID,connIter){
        if(connIter.ent != *ENTFROMCOMP(conn)){
            Sendf(connIter.ptr,msg,"%s connected!",conn->username);
        }
    }
}
