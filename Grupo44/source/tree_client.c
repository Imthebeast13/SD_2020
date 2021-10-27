#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <signal.h>
#include "network_client.h"
#include "sdmessage.pb-c.h"
#include "data.h"
#include "entry.h"
#include "client_stub.h"


int main(int argc, char **argv) {
    char *value;
    struct rtree_t *rtree;
    signal(SIGPIPE, SIG_IGN);


    if (argc != 2) {   // Allow one or two integers
        printf("Forma de utilizar: ./tree-client <ip_server>:<port_server> \n");
        printf("Exemplo de uso: ./tree-client 127.0.0.1:12345 \n");
        return -1;
    }
    char *tkn;

        if ((rtree = rtree_connect(argv[1])) == NULL) {
            exit(-1);
        }
    printf("Já estah ligado!\n");
    while (1) {
        printf("Insira o comando pretendido\n");
        value = malloc(MAX_MSG);
        fgets(value, MAX_MSG, stdin);
        if (value[strlen(value) - 1] == '\n')
            value[strlen(value) - 1] = '\0';
        tkn = strtok(value, " ");

        if (strcmp(value, "quit") == 0) {
            printf("Fim do Programa!!! \n");
            free(tkn);
            break;

        }
        char *arg2;
        if (strcmp(tkn, "put") == 0) {
            int resultado = 0;
            arg2 = strtok(NULL, " ");
            char *arg3 = strtok(NULL, " ");
            if (arg2 == NULL || arg3 == NULL) {
                printf("Não inseriu argumentos adequados; O input tem de ser do tipo: put chave valor\n");
                printf("Exemplo de uso: put a b\n");
            } else if ((resultado = rtree_put(rtree, entry_create(strdup(arg2), data_create2(strlen(arg3), arg3)))) <
                       0) {
                printf("Não foi possivel realizar o put\n");

            } else {
                printf("A sua operacao eh a operacao num %d\n",resultado);
            }

        } else if (strcmp(tkn, "get") == 0) {
            struct data_t *data;
            arg2 = strtok(NULL, " ");
            if (arg2 == NULL) {
                printf("Não inseriu argumentos adequados; O input tem de ser do tipo: get chave\n");
                printf("Exemplo de uso: get key \n");
            } else if ((data = rtree_get(rtree, arg2)) == NULL) {
                printf("Essa chave não existe!\n");
                data_destroy(data);
            }
            else {
                printf("A data correspondente ah chave %s eh %s\n", arg2, (char *) data->data);
                data_destroy(data);
            }
        } else if (strcmp(tkn, "del") == 0) {
            int del;
            arg2 = strtok(NULL, " ");
            if (arg2 == NULL) {
                printf("Não inseriu argumentos adequados; O input tem de ser do tipo: del chave\n");
                printf("Exemplo de uso: del key \n");
            } else if ((del = rtree_del(rtree, arg2)) == -1) {
                printf("Não existe o que pretende eliminar\n");
            } else {
                printf("A sua operacao eh a operacao num %d\n",del);
            }

        } else if (strcmp(tkn, "size") == 0) {
            int size;
            if ((size = rtree_size(rtree)) == -1) {
                printf("A Arvore nao existe!\n ");
            } else {
                printf("%d\n", size);
            }


        } else if (strcmp(tkn, "height") == 0) {
            int height;
            if ((height = rtree_height(rtree)) == -1) {
                printf("A Arvore nao existe!\n ");
            } else {
                printf("%d\n", height);
            }

        } else if (strcmp(tkn, "getkeys") == 0) {
            char **getKeys;
            if ((getKeys = rtree_get_keys(rtree)) == NULL) {
                printf("A Arvore estah vazia!!\n");

            }
            else if ((getKeys = rtree_get_keys(rtree)) == NULL) {
                printf("A Arvore estah vazia!!\n");

            }
            else {
                if (strcmp(getKeys[0],"Nao pode fazer pedidos")==0){
                    printf( "Nao pode fazer pedidos pois um server nao esta ativo!\n");
                }
                else {
                    for (int i = 0; getKeys[i] != NULL; i++) {
                        printf("chave: %s\n", getKeys[i]);
                    }
                    rtree_free_keys(getKeys);

                }

            }
        } else if(strcmp(tkn,"verify")==0){
            int contains;
            arg2 = strtok(NULL, " ");
            if(arg2==NULL){
                printf("Nao inseriu nenhuma operação!O argumento tem de ser do tipo: verify <op_num>\n");
            }
            else if ((contains=rtree_verify(rtree,atoi(arg2)))!=0){
                printf("O seu pedido ainda nao foi realizado :-/\n");
            }
            else{
                printf("O seu pedido jah foi realizado!\n");
            }
        }
        else {
            printf("O comando que inseriu eh invalido! (Comandos validos:put,get,del,size,height,getKeys)");
        }

        free(value);
        /*if ((rtree = rtree_connect(argv[1])) == NULL) {
            exit(-1);
        }*/

    }
    printf("A ligacao com o server terminou . Adeus! \n");
    rtree_disconnect(rtree);

}

//void kill_client(){
//  rtree_disconnect(rtree);
//free(value);
//    exit(1);
//}
