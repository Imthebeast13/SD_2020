#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include "client_stub-private.h"
#include "message-private.h"
#include "network_client.h"
#include "client_stub.h"
#include "../include/client_stub-private.h"
#include <zookeeper/zookeeper.h>
#include "network_server.h"
static int is_connected;
static zhandle_t * zh;
typedef struct String_vector zoo_string;
static char *root_path = "/kvstore";
static char *root_primary = "/kvstore/primary";
static char *root_backup = "/kvstore/backup";
static char *watcher_ctx = "ZooKeeper Data Watcher";
char*dados_primary;
char*dados_backup;
struct rtree_t *rtree;
#define ZDATALEN 1024 * 1024
int flag_alteracao=0;
int flag_isInitial=0;
/* Função para estabelecer uma associação entre o cliente e o servidor,
* em que address_port é uma string no formato <hostname>:<port>.
* Retorna NULL em caso de erro.
*/

static void child_watcher(zhandle_t *wzh, int type, int state, const char *zpath, void *watcher_ctx) {
    zoo_string *children_list = (zoo_string *) malloc(sizeof(zoo_string));
    if (state == ZOO_CONNECTED_STATE) {
        if (type == ZOO_CHILD_EVENT) {
            /* Get the updated children and reset the watch */
            if (ZOK != zoo_wget_children(zh, root_path, child_watcher, watcher_ctx, children_list)) {
                fprintf(stderr, "Error setting watch at %s!\n", root_path);
            }


            if (children_list->count==0){
                fprintf(stderr,"Nao ha servers ativos!! Adeus\n");
                exit(EXIT_FAILURE);
            }
            if (flag_alteracao>0){
                flag_alteracao--;
                if (ZOK != zoo_wget_children(zh, root_path, child_watcher, watcher_ctx, children_list)) {
                    fprintf(stderr, "Error setting watch at %s!\n", root_path);
                }
            }
            else {
                fprintf(stderr, "\nEis os servidores ativos neste momento no path %s:\n ", root_path);


                int a = 1;
                for (int i = children_list->count; i > 0; i--) {
                    fprintf(stderr, "(%d): %s\n", a, children_list->data[i - 1]);
                    a += 1;
                }

                if (children_list->count != 2) {
                    if (ZNONODE == zoo_exists(zh, root_primary, 0, NULL)) {
                        fprintf(stderr, "O main server desconectou-se! O backup up server passou a Main Server\n");

                        /*rtree->sock_primary = rtree->sock_backup;
                        rtree->endereco_primary = rtree->endereco_backup;*/
                        int size = 1024;

                        if (ZOK != zoo_get(zh, root_backup, (void *) watcher_ctx, dados_backup, &size, NULL)) {
                            fprintf(stderr, "Error setting watch at %s!\n", root_backup);
                            exit(EXIT_FAILURE);
                        }


                       /* if ((close(rtree->sock_backup)) < 0) {
                            printf("Erro a fechar a ligacao estabelecida!\n");
                        }
                        rtree->sock_backup = -1;*/
                        flag_alteracao = 2;
                        flag_isInitial=1;
                        rtree->endereco_primary.sin_port=rtree->endereco_backup.sin_port;
                        rtree->endereco_primary.sin_addr=rtree->endereco_backup.sin_addr;
                        rtree->sock_primary=rtree->sock_backup;
                        //rtree_connect(dados_backup);

                        if (ZOK != zoo_wget_children(zh, root_path, child_watcher, watcher_ctx, children_list)) {
                            fprintf(stderr, "Error setting watch at %s!\n", root_path);
                        }
                    } else {
                            fprintf(stderr, "Não há backup server! Não há mais pedidos momentaneamente\n");
                            if ((close(rtree->sock_backup)) < 0) {
                                printf("Erro a fechar a ligacao estabelecida!\n");
                            }
                            rtree->sock_backup = -1;
                        if (ZOK != zoo_wget_children(zh, root_path, child_watcher, watcher_ctx, children_list)) {
                            fprintf(stderr, "Error setting watch at %s!\n", root_path);
                        }

                    }

                }

                else{
                    int size=1024;
                    dados_backup=malloc(size);
                    if (ZOK!=zoo_get(zh, root_backup,(void *)watcher_ctx, dados_backup, &size, NULL)){
                        fprintf(stderr, "Error setting watch at %s!\n", root_primary);
                    }
                    flag_isInitial=2;
                    rtree_connect(dados_backup);

                    if (ZOK != zoo_wget_children(zh, root_path, child_watcher, watcher_ctx, children_list)) {
                        fprintf(stderr, "Error setting watch at %s!\n", root_path);
                    }

                    fprintf(stderr,"Já pode realizar pedidos novamente!\n");
                }

            }
        }
    }
}

void connection_watcher(zhandle_t *zzh, int type, int state, const char *path, void* context) {
    if (type == ZOO_SESSION_EVENT) {
        if (state == ZOO_CONNECTED_STATE) {
            is_connected = 1;
        } else {
            is_connected = 0;
        }
    }
}

struct rtree_t *rtree_connect(const char *ip_and_zookeeper) {
    if (flag_isInitial == 0) {
        char arg2[120] = "localhost:";
        char *zookeeper = strtok(strdup(ip_and_zookeeper), ":");
        zookeeper = strtok(NULL, ":");
        strcat(arg2, zookeeper);
        zh = zookeeper_init(arg2, connection_watcher, 2000, 0, 0, 0);
        if (zh == NULL) {
            fprintf(stderr, "Nao me consegui conectar ao servidor ZooKeeper [%d]!\n", errno);
            exit(EXIT_FAILURE);
        }
        zoo_string *children_list = (zoo_string *) malloc(sizeof(zoo_string));
        if (ZOK != zoo_wget_children(zh, root_path, child_watcher, watcher_ctx, children_list)) {
            fprintf(stderr, "Error setting watch at %s!\n", root_path);
        }

        char *ip_and_zoo_copy = strdup(ip_and_zookeeper);
        rtree = (struct rtree_t *) malloc(sizeof(struct rtree_t));
        char *host = strtok(ip_and_zoo_copy, ":");


        rtree->endereco_primary.sin_family = AF_INET;
        rtree->endereco_backup.sin_family = AF_INET;
        int size = 1024;
        dados_primary = malloc(size);
        if (ZOK != zoo_get(zh, root_primary, (void *) watcher_ctx, dados_primary, &size, NULL)) {
            fprintf(stderr, "Error setting watch at %s!\n", root_primary);
            exit(EXIT_FAILURE);
        }
        dados_backup = malloc(size);
        if (ZOK != zoo_get(zh, root_backup, (void *) watcher_ctx, dados_backup, &size, NULL)) {
            fprintf(stderr, "Error setting watch at %s!\n", root_backup);
            exit(EXIT_FAILURE);
        }

        char *port_primary = strtok(strdup(dados_primary), ":");
        port_primary = strtok(NULL, ":");
        rtree->endereco_primary.sin_port = htons(atoi(port_primary));
        if (inet_pton(AF_INET, host, &(rtree->endereco_primary.sin_addr)) < 1) {
            printf("Erro a converter IP primary\n");
            close(rtree->sock_primary);
            return NULL;
        }
        char *port_backup = strtok(strdup(dados_backup), ":");
        port_backup = strtok(NULL, ":");

        rtree->endereco_backup.sin_port = htons(atoi(port_backup));
        if (inet_pton(AF_INET, host, &(rtree->endereco_backup.sin_addr)) < 1) {
            printf("Erro a converter IP backup\n");
            close(rtree->sock_backup);
            return NULL;
        }

        free(ip_and_zoo_copy);

        if (network_connect(rtree) < 0) {
            printf("Impossivel conectar!!\n");
            free(rtree);
            return NULL;
        }
        return rtree;
    }
    else if (flag_isInitial == 1) {
        char *port_primary = strtok(strdup(ip_and_zookeeper), ":");
        port_primary = strtok(NULL, ":");
        char *ip_and_zoo_copy = strdup(ip_and_zookeeper);
        rtree->endereco_primary.sin_port = htons(atoi(port_primary));
        char *host = strtok(ip_and_zoo_copy, ":");
        if (inet_pton(AF_INET, host, &(rtree->endereco_primary.sin_addr)) < 1) {
            printf("Erro a converter IP primary\n");
            close(rtree->sock_primary);
            return NULL;
        }
        if (network_connect(rtree) < 0) {
            printf("Impossivel conectar!!\n");
            free(rtree);
            return NULL;
        }
        return 0;
    } else {
        char *port_backup = strtok(strdup(ip_and_zookeeper), ":");
        port_backup = strtok(NULL, ":");
        char *ip_and_zoo_copy = strdup(ip_and_zookeeper);
        rtree->endereco_backup.sin_port = htons(atoi(port_backup));
        char *host = strtok(ip_and_zoo_copy, ":");
        if (inet_pton(AF_INET, host, &(rtree->endereco_backup.sin_addr)) < 1) {
            printf("Erro a converter IP primary\n");
            close(rtree->sock_backup);
            return NULL;
        }
        if (network_connect2(rtree) < 0) {
            printf("Impossivel conectar!!\n");
            free(rtree);
            return NULL;
        }
        return 0;
    }
}
/* Termina a associação entre o cliente e o servidor, fechando a 
 * ligação com o servidor e libertando toda a memória local.
 * Retorna 0 se tudo correr bem e -1 em caso de erro.
 */
int rtree_disconnect(struct rtree_t *rtree) {
    free(&rtree->endereco_primary);
    free(&rtree->endereco_backup);

    return network_close(rtree);

}

/* Função para adicionar um elemento na árvore.
 * Se a key já existe, vai substituir essa entrada pelos novos dados.
 * Devolve 0 (ok, em adição/substituição) ou -1 (problemas).
 */
int rtree_put(struct rtree_t *rtree, struct entry_t *entry) {
    if(ZNONODE!=zoo_exists(zh,root_backup,0,NULL) && ZNONODE!=zoo_exists(zh,root_primary,0,NULL)) {
        struct message_t *message = malloc(sizeof(struct message_t));
        MessageT messag;
        message_t__init(&messag);
        message->message = &messag;

        //message->message = &messag;
        message->message->opcode = OP_PUT;
        message->message->c_type = CT_ENTRY;
        message->message->key = entry->key;
        message->message->data = entry->value->data;
        message->message->data_size = entry->value->datasize;
        struct message_t *msg = network_send_receive(rtree, message);
        if (msg == NULL) {
            network_close(rtree);
            return -1;
        }
        if (msg->message->opcode == OP_ERROR) {
            //free(message);
            free(msg);
            return -1;
        }
        if (msg->message->opcode == OP_PUT + 1) {
            //free(message);
            //free(msg);
            return msg->message->data_size;;
        }
        return -1;
    }
    else {
        fprintf(stderr, "Nao pode fazer pedidos pois um server nao esta ativo!\n");
        return -1;
    }
}

/* Função para obter um elemento da árvore.
 * Em caso de erro, devolve NULL.
 */
struct data_t *rtree_get(struct rtree_t *rtree, char *key) {
    if (ZNONODE != zoo_exists(zh, root_backup, 0, NULL) && ZNONODE != zoo_exists(zh, root_primary, 0, NULL)) {
        struct message_t *message = malloc(sizeof(struct message_t));
        MessageT messag;
        message_t__init(&messag);
        message->message = &messag;
        message->message->opcode = OP_GET;
        message->message->c_type = CT_KEY;
        message->message->key = key;
        struct message_t *msg = network_send_receive(rtree, message);
        if (msg->message->opcode == OP_GET + 1) {

            if (strcmp(msg->message->data, "") == 0 && (msg->message->data_size) == 1) {
                //free(msg);
                return NULL;
            }
            struct data_t *data = data_create2(msg->message->data_size, msg->message->data);
            //free(msg);
            return data;
        }
        //free(msg);
        return NULL;
    } else {
        fprintf(stderr, "Nao pode fazer pedidos pois um server nao esta ativo!\n");
        return -1;
    }
}

/* Função para remover um elemento da árvore. Vai libertar 
 * toda a memoria alocada na respetiva operação rtree_put().
 * Devolve: 0 (ok), -1 (key not found ou problemas).
 */
int rtree_del(struct rtree_t *rtree, char *key) {
    if (ZNONODE != zoo_exists(zh, root_backup, 0, NULL) && ZNONODE != zoo_exists(zh, root_primary, 0, NULL)) {
        struct message_t *message = malloc(sizeof(struct message_t));
        MessageT messag;
        message_t__init(&messag);
        message->message = &messag;
        message->message->opcode = OP_DEL;
        message->message->c_type = CT_KEY;
        message->message->key = key;
        struct message_t *msg = network_send_receive(rtree, message);
        if (msg->message->opcode == OP_ERROR) {
            free(msg);
            return -1;
        }
        if (msg->message->opcode == OP_DEL + 1) {
            return msg->message->data_size;
        }
        free(msg);
        return -1;
    } else {
        fprintf(stderr, "Nao pode fazer pedidos pois um server nao esta ativo!\n");
        return -1;
    }
}

/* Devolve o número de elementos contidos na árvore.
 */
int rtree_size(struct rtree_t *rtree) {
    if (ZNONODE != zoo_exists(zh, root_backup, 0, NULL) && ZNONODE != zoo_exists(zh, root_primary, 0, NULL)) {
        struct message_t *message = malloc(sizeof(struct message_t));
        MessageT messag;
        message_t__init(&messag);
        message->message = &messag;
        message->message->opcode = OP_SIZE;
        message->message->c_type = CT_NONE;
        struct message_t *msg = network_send_receive(rtree, message);
        int size = msg->message->data_size;
        //free(msg);
        free(message);
        return size;
    }
    else {
    fprintf(stderr, "Nao pode fazer pedidos pois um server nao esta ativo!\n");
    return -1;
    }
}

/* Função que devolve a altura da árvore.
 */
int rtree_height(struct rtree_t *rtree) {
    if (ZNONODE != zoo_exists(zh, root_backup, 0, NULL) && ZNONODE != zoo_exists(zh, root_primary, 0, NULL)) {
        struct message_t *message = malloc(sizeof(struct message_t));
    MessageT messag;
    message_t__init(&messag);
    message->message=&messag;
    message->message->opcode = OP_HEIGHT;
    message->message->c_type = CT_NONE;
    struct message_t *msg = network_send_receive(rtree, message);
    int height = msg->message->data_size;
    //free(msg);
    free(message);
    return height;
}
    else {
        fprintf(stderr, "Nao pode fazer pedidos pois um server nao esta ativo!\n");
        return -1;
    }
}
/* Devolve um array de char* com a cópia de todas as keys da árvore,
 * colocando um último elemento a NULL.
 */
char **rtree_get_keys(struct rtree_t *rtree) {
    if (ZNONODE != zoo_exists(zh, root_backup, 0, NULL) && ZNONODE != zoo_exists(zh, root_primary, 0, NULL)) {
        struct message_t *message = malloc(sizeof(struct message_t));
    MessageT messag;
    message_t__init(&messag);
    message->message=&messag;
    message->message->opcode = OP_GETKEYS;
    message->message->c_type = CT_NONE;
    struct message_t *msg = network_send_receive(rtree, message);
    if (msg->message->opcode == OP_ERROR) {
        free(msg);
        //free(message);
        return NULL;
    }
    if (msg->message->opcode == OP_GETKEYS + 1) {
        char *chave_junta = msg->message->key;
        int tamanho=0;
        int a=0;
        int *pointer=&a;
        char* token=strtok(strdup(chave_junta),"/");
        while(token!=NULL){
            token=strtok(NULL,"/");
            tamanho++;

    }
        if(tamanho==0){
            return NULL;
        }
        char**keys= (char **)malloc(sizeof(char*)*tamanho+1);
        keys[tamanho]=NULL;
        char*token2=strtok(strdup(chave_junta),"/");
            while ((*pointer)<tamanho){
                keys[*pointer]=strdup(token2);
                token2=strtok(NULL,"/");
                (*pointer)++;
            }

        free(msg);
        return keys;
    }
    return NULL;
}
    else {
        char**a= (char **)malloc(sizeof(char*)*20);
        a[0]="Nao pode fazer pedidos";
        return a;
    }
}
/* Liberta a memória alocada por rtree_get_keys().
 */
void rtree_free_keys(char **keys) {
    int a = 0;
    while (keys[a]!= NULL) {
        free(keys[a]);
        a++;
    }


    free(keys);

}

int rtree_verify(struct rtree_t *rtree,int op_n){
    if (ZNONODE != zoo_exists(zh, root_backup, 0, NULL) && ZNONODE != zoo_exists(zh, root_primary, 0, NULL)) {
        struct message_t *message = malloc(sizeof(struct message_t));
    MessageT messag;
    message_t__init(&messag);
    message->message=&messag;
    message->message->opcode = OP_VERIFY;
    message->message->c_type = CT_RESULT;
    message->message->data_size=op_n;
    struct message_t *msg = network_send_receive(rtree, message);
    if (msg->message->opcode==OP_ERROR){
        free(message);
        return -1;
    }
    free(message);
    return 0;
}
    else {
        fprintf(stderr, "Nao pode fazer pedidos pois um server nao esta ativo!\n");
        return -1;
    }
}
    //free(msg);



