#include "message-private.h"
#include "network_server.h"
#include "inet.h"
#include<sys/types.h>
#include<sys/socket.h>
#include <poll.h>
#define TIMEOUT 50
#define NFDESC 500
int saveSock;
struct pollfd connections[NFDESC];
struct sockaddr_in server,cliente;
int network_server_init(short port){
    //socklen_t tamanho_cliente;
        if((saveSock=socket(AF_INET,SOCK_STREAM,0))<0){
            perror("Nao foi possivel criar o server!!\n");
            return -1;
        }

        server.sin_family=AF_INET;
        server.sin_port=htons(port);
        server.sin_addr.s_addr=htonl(INADDR_ANY);

        if(bind(saveSock,(struct sockaddr*) &server, sizeof(server))<0){
            perror("Erro a fazer bind!\n");
            close(saveSock);
            return -1;
        }

        if(listen(saveSock,0)<0){
            perror("Erro a escutar!\n");
            close(saveSock);
            return -1;
        }
        printf("O server estah a espera que lhe envie dados!\n");
        return saveSock;
}
int network_main_loop(int listening_socket) {
    int nfds,kfds;
    for (int i = 0; i < NFDESC; i++)
        connections[i].fd= -1;
    connections[0].fd = saveSock;
    connections[0].events=POLLIN;

    nfds=1;

    saveSock=listening_socket;
    struct message_t *message;
    socklen_t tamanho=sizeof(struct sockaddr);
    //int sockValue;
    while ((kfds=poll(connections,nfds,TIMEOUT))>=0)
        if (kfds > 0) {
            if ((connections[0].revents & POLLIN) && (nfds < NFDESC))
                if ((connections[nfds].fd = accept(connections[0].fd, (struct sockaddr *) &cliente, &tamanho)) > 0) {
                connections[nfds].events = POLLIN;
                nfds += 1;
            }

            for (int i = 1; i < nfds; ++i) {
                if (connections[i].revents & POLLIN) {
                    if ((message = network_receive(connections[i].fd)) == NULL) {
                        perror("Erro ao receber dados do cliente");
                        close(connections[i].fd);
                        connections[i].fd = -1;
                        continue;
                    } else {
                        if(message->message->opcode==MESSAGE_T__OPCODE__OP_BAD){
                            close(connections[i].fd);
                            connections[i].fd = -1;
                            continue;
                        }

                        invoke_aux(listening_socket,message);

                        invoke(message);
                        if ((network_send(connections[i].fd, message))<0){
                            perror("Erro ao enviar resposta ao cliente");
                            close(connections[i].fd);
                            connections[i].fd = -1;
                            continue;
                        }

                    }

                }
                if ((connections[i].revents & POLLHUP) || (connections[i].revents & POLLERR)) {
                    close(connections[i].fd);
                    connections[i].fd = -1;
                    continue;
                }
            }
        }
            network_server_close();
            return 0;
         }

    /*socklen_t tamanho = sizeof(struct sockaddr);
    struct message_t *message;
    int sockValue;
        while ((sockValue = accept(listening_socket, (struct sockaddr *) &cliente, &tamanho)) != -1) {
        if ((message = network_receive(sockValue)) != NULL) {
            if ((invoke(message)) != 0) {
                printf("Não foi possível realizar o pretendido\n");
                //destroy_message(message);
            }
            network_send(sockValue, message);
        }
    }

    close(sockValue);

}*/



struct message_t *network_receive(int client_socket){
    int size_of_message=0;

    if(read_All(client_socket,(char*)&size_of_message,sizeof(int))<0) {
        return NULL;
    }
    uint8_t * received= malloc(size_of_message);
        if(read_All(client_socket,(char*)received,size_of_message)<0){
        return NULL;
    }
        MessageT * messageT;
//size_t length= strlen(received);
    //uint8_t buf[length];
    //memcpy(buf,received,length+1);
    messageT=message_t__unpack(NULL,size_of_message,received);
    struct message_t *resposta=malloc(sizeof(struct message_t));
    resposta->message=messageT;
    /*
    mensagem->opCode =message->opcode;
    mensagem->cType=message->c_type;
    mensagem->conteudo.key=message->key;
    mensagem->conteudo.value->data=message->data;
    mensagem->conteudo.value->datasize=message->data_size;*/

    //free(integer);
    return resposta;

}
int network_send(int client_socket, struct message_t *msg){
    int size=0;
    /*MessageT message;
    message_t__init(&message);
    message.c_type=msg->cType;
    message.opcode=msg->opCode;
    message.key=msg->conteudo.key;
    message.data_size=msg->conteudo.value->datasize;
    message.data=msg->conteudo.value->data;*/

    int len = message_t__get_packed_size(msg->message);
    void *buf = malloc(len);
    message_t__pack(msg->message,buf);
    if((size = write_All(client_socket,buf,len))!=len){
        printf("Erro a enviar dados do Server!\n");
        close(client_socket);
        return -1;
}
    free(buf);
    free(msg->message);
    free(msg);
    return 0;
}

int network_server_close(){
if(close(saveSock)!=0){
    printf("Erro a encerrar este Socket\n");
    return -1;
}
    return 0;
}

