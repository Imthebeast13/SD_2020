#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <signal.h>

#include "data.h"
#include "entry.h"
#include "tree.h"
#include "message-private.h"
#include "tree_skel.h"
#include "network_server.h"

int sock;
int main(int argc,char** argv) {
    signal(SIGPIPE,SIG_IGN);
    if (argc != 3) {
        printf("Forma de utilizar:: ./tree-server socket <ipaddress>:<porto>\n");
        printf("Exemplo de uso: ./tree-server 12345 127.0.01:2181  \n");
        return -1;
    }

    if ((sock = network_server_init(atoi(argv[1]))) == -1) {
        printf("Impossivel criar server\n");
    }

    if (tree_skel_init_private(argv[1],argv[2]) < 0) {
        printf("Impossivel criar a Arvore\n");
    }
    if (network_main_loop(sock) == -1) {
        printf("Erro a fazer Main Loop\n");
    }
    tree_skel_destroy();
    printf("A desligar Server\n");
    network_server_close();
    return 0;
}

//void killServer(){
  //  tree_skel_destroy();
    //network_server_close();
    //exit(1);
//}