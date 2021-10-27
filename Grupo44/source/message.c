#include "message-private.h"


int write_All(int sock, char  *buf, int len) {
    //int bufsize= len;
    //while(len>0) {
        int res = write(sock, buf, len);
        if(res<0) {
            if(errno==EINTR) ;
            perror("write failed:");
            return -1;
        }
        /* buf+= res;
         len-= res;*/
    //}
    return res;
}

int read_All(int sock, char *buf, int len) {
    //int bufsize = len;
    int res=0;
    //while(len>0) {
         res=read(sock, buf, len);
        if(res<0) {
            if(errno==EINTR);
            perror("read failed:");
            return -1;
        //}
        /*buf += res;
        len-= res;*/
    }
    return res;




}

/*void destroy_message(struct message_t *message){
    if (message==NULL)
    return;

    short type=message->cType;

    if(type==CT_KEY){
        free(message->conteudo.key);
    }
    else if(type==CT_VALUE){
    data_destroy(message->conteudo.value);

    }
    else if(type==CT_ENTRY){
        entry_destroy(message->conteudo.entry);
    }
    else if(type==CT_KEYS){
        tree_free_keys(message->conteudo.keys);
    }
    free(message);

}*/
