#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include "inet.h"
#include<sys/types.h>
#include<sys/socket.h>
#include "network_client.h"
#include "client_stub.h"
#include "client_stub-private.h"
int flag=0;


int network_connect(struct rtree_t *rtree) {
    if (flag == 0) {
        flag=1;
        int inicial_primary;
        if ((inicial_primary = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("Nao foi possivel criar o Cliente!!\n");
            return -1;
        }
        rtree->sock_primary = inicial_primary;


        if (connect(rtree->sock_primary, (struct sockaddr *) &(rtree->endereco_primary),
                    sizeof(rtree->endereco_primary)) < 0) {
            printf("Nao foi possivel conectar-se ao servidor 1\n");
            close(rtree->sock_primary);
            return -1;
        }
        int inicial_backup;
        if ((inicial_backup = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("Nao foi possivel criar o Cliente!!\n");
            return -1;
        }
        rtree->sock_backup = inicial_backup;


        if (connect(rtree->sock_backup, (struct sockaddr *) &(rtree->endereco_backup), sizeof(rtree->endereco_backup)) <
            0) {
            fprintf(stderr,"Nao foi possivel conectar-se ao servidor 2\n");
            close(rtree->sock_backup);
            return -1;
        }
        return 0;
    }
    else{
        int inicial_primary;
        if ((inicial_primary = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("Nao foi possivel criar o Cliente!!\n");
            return -1;
        }
        rtree->sock_primary = inicial_primary;


        if (connect(rtree->sock_primary, (struct sockaddr *) &(rtree->endereco_primary),
                    sizeof(rtree->endereco_primary)) < 0) {
            printf("Nao foi possivel conectar-se ao servidor 111\n");
            close(rtree->sock_primary);
            return -1;
        }
        return 0;
    }
}
int network_connect2(struct rtree_t *rtree) {
    int inicial_backup;
    if ((inicial_backup = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Nao foi possivel criar o Cliente!!\n");
        return -1;
    }
    rtree->sock_backup = inicial_backup;


    if (connect(rtree->sock_backup, (struct sockaddr *) &(rtree->endereco_backup),
                sizeof(rtree->endereco_backup)) < 0) {
        printf("Nao foi possivel conectar-se ao servidor 2222\n");
        close(rtree->sock_backup);
        return -1;
    }
    return 0;
}

/* Esta função deve:
 * - Obter o descritor da ligação (socket) da estrutura rtree_t;
 * - Serializar a mensagem contida em msg;
 * - Enviar a mensagem serializada para o servidor;
 * - Esperar a resposta do servidor;
 * - De-serializar a mensagem de resposta;
 * - Retornar a mensagem de-serializada ou NULL em caso de erro.
 */
struct message_t *network_send_receive(struct rtree_t * rtree,struct message_t *msg){
    //message_t__init(msg->message);
    int size;
    //message_t__init(msg->message);

    /* MessageT message;
     message_t__init(&message);
     message.c_type=msg->cType;
     message.opcode=msg->opCode;
     message.key=msg->conteudo.key;
     message.data_size=msg->conteudo.value->datasize;
     message.data=msg->conteudo.value->data;
     */

    int len = message_t__get_packed_size(msg->message);
    void *buf = malloc(len);
    void *buf2 = malloc(MAX_MSG);

    if (msg->message->opcode==OP_PUT || msg->message->opcode==OP_DEL) {
        message_t__pack(msg->message, buf);


        if ((size = write_All(rtree->sock_primary, (char *) &len, sizeof(int))) != sizeof(int)) {
            printf("Erro a enviar dados!\n");
            close(rtree->sock_primary);
            free(buf);
            return NULL;
        }

        if ((size = write_All(rtree->sock_primary, buf, len)) != len) {
            printf("Erro a enviar dados!\n");
            close(rtree->sock_primary);
            free(buf);
            return NULL;
        }



        if ((size = read_All(rtree->sock_primary, buf2, MAX_MSG)) < 0) {
            printf("Erro a ler dados!\n");
            //free(buf2);
            free(buf);
            return NULL;

        }

        msg->message=message_t__unpack(NULL,size,buf2);

    }
    else{
        message_t__pack(msg->message,buf);
        if((size = write_All(rtree->sock_backup,(char*)&len,sizeof(int)))!=sizeof(int)){
            printf("Erro a enviar dados!\n");
            close(rtree->sock_backup);
            free(buf);
            return NULL;
        }
        if((size = write_All(rtree->sock_backup,buf,len))!=len){
            printf("Erro a enviar dados!\n");
            close(rtree->sock_backup);
            free(buf);
            return NULL;
        }

        if ((size = read_All(rtree->sock_backup, buf2, MAX_MSG)) < 0) {
            printf("Erro a ler dados!\n");
            //free(buf2);
            free(buf);
            return NULL;

        }
        msg->message=message_t__unpack(NULL,size,buf2);

    }






    free(buf2);
    free(buf);
    return msg;


}







int network_close(struct rtree_t * rtree){
    if((close(rtree->sock_primary))<0){
        printf("Erro a fechar a ligacao estabelecida!\n");
        return -1;
    }
    if((close(rtree->sock_backup))<0){
        printf("Erro a fechar a ligacao estabelecida!\n");
        return -1;
    }
    rtree->sock_primary=-1;
    rtree->sock_backup=-1;
    return 0;
}
