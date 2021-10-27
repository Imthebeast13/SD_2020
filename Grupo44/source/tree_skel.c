#include <tree-private.h>
#include <pthread.h>
#include <zookeeper/zookeeper.h>
#include<sys/socket.h>
#include "message-private.h"
#include "tree_skel.h"
#include "stdio.h"
#include "stdlib.h"
#include "network_server.h"
#include "client_stub.h"
#include "network_client.h"
#include "client_stub-private.h"
static zhandle_t * zh;
int last_assigned=0;
int op_count=0;
int count_servers=0;
char* save_new_root;
char node_path[120] = "";
static char *root_path = "/kvstore";
static char *root_primary = "/kvstore/primary";
static char *root_backup = "/kvstore/backup";
struct tree_t *tree;
struct task_t*queue_head=NULL;
static int is_connected;
static char *watcher_ctx = "ZooKeeper Data Watcher";
typedef struct String_vector zoo_string;
pthread_cond_t queue_not_empty=PTHREAD_COND_INITIALIZER;
pthread_mutex_t queue_lock,tree_lock=PTHREAD_MUTEX_INITIALIZER;
char* dados;
char*dados_main;
#define ZDATALEN 1024 * 1024
int flag_ativas=0;
int flag_alteracao=0;
char * repete_put;
struct rtree_t*rtree;

void conecta(struct rtree_t* rtree){
    int size=1024;
    repete_put=malloc(size);
    if (ZOK!=zoo_get(zh, root_backup,(void *)watcher_ctx, repete_put, &size, NULL)){
        fprintf(stderr, "Ups\n");
    }
    char *host=strtok(strdup(repete_put),":");
    repete_put=strtok(repete_put,":");
    repete_put=strtok(NULL,":");
    rtree->endereco_primary.sin_family = AF_INET;
    rtree->endereco_primary.sin_port=htons(atoi(repete_put));
    if (inet_pton(AF_INET, host, &(rtree->endereco_primary.sin_addr)) < 1) {
        printf("Erro a converter IP primary\n");
        close(rtree->sock_primary);
        exit(EXIT_FAILURE);
    }
    if (private_connect(rtree) < 0) {
        printf("Impossivel conectar!!\n");
        free(rtree);
        exit(EXIT_FAILURE);
    }


}

int private_connect(struct rtree_t* rtree){
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
    fprintf(stderr,"Conectei me ao server backup!\n");
    return 0;
}





static void child_watcher(zhandle_t *wzh, int type, int state, const char *zpath, void *watcher_ctx) {
    zoo_string* children_list =	(zoo_string *)malloc(sizeof(zoo_string));
    if (state == ZOO_CONNECTED_STATE)	 {
        if (type == ZOO_CHILD_EVENT) {
            /* Get the updated children and reset the watch */
            if (ZOK != zoo_wget_children(zh, root_path, child_watcher, watcher_ctx, children_list)) {
                fprintf(stderr, "Error setting watch at %s!\n", root_path);
            }
            fprintf(stderr, "\nEis os servidores ativos neste momento no path %s:\n ", root_path);


            int a=1;
            for (int i = children_list->count; i > 0; i--)  {
                fprintf(stderr, "(%d): %s\n", a, children_list->data[i-1]);
                a+=1;
            }

             if(children_list->count!=2){
                 close(rtree->sock_primary);
                 if(ZNONODE==zoo_exists(zh,root_primary,0,NULL)) {
                    fprintf(stderr, "O main server desconectou-se! O backup up server passou a Main Server e os pedidos estao bloqueados momentaneamente");
                    flag_ativas=1;
                    int size=1024;
                    char *path=malloc(size);
                    dados=malloc(size);
                    if (ZOK!=zoo_get(zh, root_backup,(void *)watcher_ctx, dados, &size, NULL)){
                        fprintf(stderr, "Error setting watch at %s!\n", root_primary);
                    }
                        if (ZOK != zoo_create(zh, root_primary, dados , strlen(dados)*sizeof(char*),& ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL , path, size)) {
                        fprintf(stderr, "Error creating znode primary1 from path %s!\n", node_path);
                        exit(EXIT_FAILURE);
                    }
                    zoo_delete(zh, root_backup, 0);
                    flag_alteracao=1;
                }
                 else {
                    if (flag_alteracao==1){
                        flag_alteracao=0;
                    }
                    else
                        fprintf(stderr, "Não ha backup server! Não há mais pedidos momentaneamente");

                }
            }

             else{
                 int size=1024;
                 dados=malloc(size);
                 if (ZOK!=zoo_get(zh, root_backup,(void *)watcher_ctx, dados, &size, NULL)){
                     fprintf(stderr, "Error setting watch at %s!\n", root_primary);
                 }
                 if (ZOK != zoo_wget_children(zh, root_path, child_watcher, watcher_ctx, children_list)) {
                     fprintf(stderr, "Error setting watch at %s!\n", root_path);
                 }
                 dados_main=malloc(size);
                 if (ZOK!=zoo_get(zh, root_primary,(void *)watcher_ctx, dados_main, &size, NULL)){
                     fprintf(stderr, "Error setting watch at %s!\n", root_primary);
                 }
                 conecta(rtree);
                 op_count=0;
                 last_assigned=0;

             }
        }
    }
    free(children_list);
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

 /*void watch_children(){
    zoo_string* children_list =	NULL;
    children_list =	(zoo_string *) malloc(sizeof(zoo_string));
    if (ZOK != zoo_wget_children(zh, root_path, &child_watcher, watcher_ctx, children_list)) {
        fprintf(stderr, "Error setting watch at %s!\n", root_path);
    }
    fprintf(stderr, "\nEis os servidores ativos neste momento no path %s:\n ", root_path);



    for (int i = 0; i < children_list->count; i++)  {
        fprintf(stderr, "(%d): %s\n", i+1, children_list->data[i]);
    }
    if (children_list->count==2 && count_servers==0){
        count_servers++;
    }
    else if(children_list->count!=2 && count_servers!=0 ){
        if (strcmp(children_list->data[0],"backup")==0)
            fprintf(stderr, "O main server desconectou-se! O backup up server passou a Main Server");
        else
            fprintf(stderr, "O backup server desconectou-se! Não há mais pedidos momentaneamente");
    }
}*/
 int tree_skel_init() {
     if ((tree = tree_create()) != NULL) {
         pthread_t thread;
         if ((pthread_create(&thread,NULL,&process_task,(void*)queue_head)) != 0){
             perror("\nThread não criada.\n");
             exit(EXIT_FAILURE);
         }
         return 0;
     }
     return -1;

 }

int tree_skel_init_private(char* argSocket,char*arg1) {
     rtree= (struct rtree_t *) malloc(sizeof(struct rtree_t));
     char arg2[120] ="localhost:";
        char*arg3=strtok(strdup(arg1),":");
        arg3=strtok(NULL,":");
        strcat(arg2,arg3);
        zh=zookeeper_init(arg2,connection_watcher,2000,0,0,0);
        if(zh==NULL){
            fprintf(stderr,"Nao me consegui conectar ao servidor ZooKeeper [%d]!\n", errno);
            exit(EXIT_FAILURE);
        }
        if (ZNONODE == zoo_exists(zh, root_path, 0, NULL)) {
            fprintf(stderr, "%s doesn't exist,but we'll create it for you!.\n", root_path);
            strcat(node_path,root_path);
            int new_path_len = 1024;
            char* new_path = malloc (new_path_len);
            if (ZOK != zoo_create(zh, root_path, "", 10, &ZOO_OPEN_ACL_UNSAFE , 0, new_path, new_path_len)) {
                fprintf(stderr, "Error creating znode kvstore from path %s!\n", node_path);
                exit(EXIT_FAILURE);
            }
            fprintf(stderr, "ZNode  kvstore created! ZNode path: %s\n", new_path);
            free(new_path);


            strcat(node_path,"/primary");
            new_path = malloc (new_path_len);
            char*ip=strtok(strdup(arg1),":");
            char server_data[120]="";
            strcat(server_data,ip);
            strcat(server_data,":");
            strcat(server_data,argSocket);
            if (ZOK != zoo_create(zh, node_path, server_data , strlen(server_data)*sizeof(char*),& ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL , new_path, new_path_len)) {
                fprintf(stderr, "Error creating znode primary1 from path %s!\n", node_path);
                exit(EXIT_FAILURE);
            }
            fprintf(stderr, "ZNode Ephemeral Primary created! ZNode path: %s\n", new_path);
            free (new_path);
            flag_ativas=1;

        }
        else if(ZNONODE==zoo_exists(zh,root_primary,0,NULL)){
            fprintf(stderr, "%s doesn't exist,but we are about to create it.\n", root_primary);
            char*ip=strtok(strdup(arg1),":");
            char server_data[120]="";
            strcat(server_data,ip);
            strcat(server_data,":");
            strcat(server_data,argSocket);
            strcat(node_path,root_primary);
            int new_path_len = 1024;
            char* new_path = malloc (new_path_len);
            if (ZOK != zoo_create(zh, node_path, server_data , strlen(server_data)*sizeof(char*), & ZOO_OPEN_ACL_UNSAFE , ZOO_EPHEMERAL, new_path, new_path_len)) {
                fprintf(stderr, "Error creating znode primary2 from path %s!\n", node_path);
                exit(EXIT_FAILURE);
            }
            fprintf(stderr, "ZNode Ephemeral Primary created! ZNode path: %s\n", new_path);
            free(new_path);
            flag_ativas=1;
        }

        else if(ZNONODE==zoo_exists(zh,root_backup,0,NULL)) {
            fprintf(stderr, "%s doesn't exist,but we are about to create it.\n", root_backup);
            char*ip=strtok(strdup(arg1),":");
            char server_data[120]="";
            strcat(server_data,ip);
            strcat(server_data,":");
            strcat(server_data,argSocket);
            strcat(node_path,root_backup);
            int new_path_len = 1024;
            char* new_path = malloc (new_path_len);
            if (ZOK != zoo_create(zh, root_backup, server_data , strlen(server_data)*sizeof(char*), & ZOO_OPEN_ACL_UNSAFE , ZOO_EPHEMERAL , new_path, new_path_len)) {
                fprintf(stderr, "Error creating znode backup from path %s!\n", node_path);
                exit(EXIT_FAILURE);
            }
            fprintf(stderr, "ZNode Ephemeral backup created! ZNode path: %s\n", new_path);
            free(new_path);
            flag_ativas=2;


        }
        else{
            fprintf(stderr, "We do not need new servers, we already have a primary and a backup one!!\n");
            exit(EXIT_FAILURE);
        }

        zoo_string* children_list =	(zoo_string *)malloc(sizeof(zoo_string));
        if (ZOK != zoo_wget_children(zh, root_path, child_watcher, watcher_ctx, children_list)) {
            fprintf(stderr, "Error setting watch at %s!\n", root_path);
            exit(EXIT_FAILURE);
        }
     if ((tree = tree_create()) != NULL) {
         pthread_t thread;
         if ((pthread_create(&thread, NULL, &process_task, (void *) queue_head)) != 0) {
             perror("\nThread não criada.\n");
             exit(EXIT_FAILURE);
         }

         return 0;
     }


    return -1;

}

void tree_skel_destroy() {
    tree_destroy(tree);
}

int invoke(struct message_t *msg) {
    if (tree == NULL) {
        return -1;
    }
        if (msg->message->opcode == OP_SIZE) {
            msg->message->opcode = OP_SIZE + 1;
            msg->message->c_type = CT_RESULT;
            pthread_mutex_lock(&tree_lock);
            msg->message->data_size = tree_size(tree);
            pthread_mutex_unlock(&tree_lock);
            return 0;
        }
        if (msg->message->opcode == OP_DEL) {
            pthread_mutex_lock(&queue_lock);
            struct task_t* task=(struct task_t*)malloc(sizeof(struct task_t));
            task->op_n=last_assigned;
            last_assigned+=1;
            task->op=0;
            task->data=NULL;
            task->key=msg->message->key;
            if (queue_head==NULL){
                queue_head=task;
                task->next=NULL;
            }
            else{
                struct task_t* tptr = queue_head;
                while (tptr->next!=NULL){
                    tptr=tptr->next;
                }
                tptr->next=task;
                task->next=NULL;
            }
            pthread_cond_signal(&queue_not_empty);
            pthread_mutex_unlock(&queue_lock);

            msg->message->data_size=last_assigned;
            msg->message->opcode=OP_DEL+1;
            msg->message->c_type=CT_RESULT;
            return 0;
            pthread_mutex_unlock(&queue_lock);
            //msg->message->c_type = CT_NONE;
            /*if ((tree_del(tree, msg->message->key)) == 0) {
                free(msg->message->key);
                msg->message->opcode = OP_DEL + 1;
                return 0;
            } else {
                //free(msg->message->key);
                msg->message->key = NULL;
                msg->message->opcode = OP_ERROR;
                return -1;*/
            //}

        }
        if (msg->message->opcode == OP_GET) {
            struct data_t *data = data_create(msg->message->data_size);
            pthread_mutex_lock(&tree_lock);
            if ((data = tree_get(tree, msg->message->key)) == NULL) {
                free(msg->message->key);
                msg->message->c_type = CT_VALUE;
                msg->message->opcode = OP_GET + 1;
                msg->message->data_size = 1;
                msg->message->data = "";
                pthread_mutex_unlock(&tree_lock);
                return 0;
            } else {
                free(msg->message->key);
                msg->message->c_type = CT_VALUE;
                msg->message->opcode = OP_GET + 1;
                msg->message->data = data->data;
                msg->message->data_size = data->datasize;
                free(data);
                pthread_mutex_unlock(&tree_lock);
                return 0;
            }
        }
        if (msg->message->opcode == OP_GETKEYS) {
            msg->message->opcode = OP_GETKEYS + 1;
            msg->message->c_type = CT_KEYS;
            pthread_mutex_lock(&tree_lock);
            char **keys = tree_get_keys(tree);

            char *chave_final = malloc(MAX_MSG * 2);
            chave_final[0] = '\0';
            int i = 0;
            if (keys != NULL) {
                while (keys[i] != NULL) {
                    strcat(chave_final, keys[i]);
                    strcat(chave_final, "/");

                    i += 1;
                }

                msg->message->key = chave_final;
                pthread_mutex_unlock(&tree_lock);
                return 0;
            }
            pthread_mutex_unlock(&tree_lock);
            return 0;

        }
        if (msg->message->opcode == OP_PUT) {
            pthread_mutex_lock(&queue_lock);
            struct task_t* task=(struct task_t*)malloc(sizeof(struct task_t));
            task->op_n=last_assigned;
            last_assigned+=1;
            task->op=1;
            task->data=msg->message->data;
            task->key=msg->message->key;
            if (queue_head==NULL){
                queue_head=task;
                task->next=NULL;
            }
            else{
                struct task_t* tptr = queue_head;
                while (tptr->next!=NULL){
                    tptr=tptr->next;
                }
                tptr->next=task;
                task->next=NULL;
            }
            pthread_cond_signal(&queue_not_empty);

            pthread_mutex_unlock(&queue_lock);

            msg->message->data_size=last_assigned;
            msg->message->opcode=OP_PUT+1;
            msg->message->c_type=CT_RESULT;
            return 0;
        }


            /*struct data_t *data = data_create2(msg->message->data_size, msg->message->data);
            //struct entry_t* entry=entry_create(msg->message->key,data);
            if ((tree_put(tree, msg->message->key, data)) == -1) {
                //entry_destroy(entry);
                data_destroy(data);
                msg->message->c_type = CT_NONE;
                msg->message->opcode = OP_ERROR;
                return 0;
            } else {
                data_destroy(data);
                msg->message->c_type = CT_NONE;
                msg->message->opcode = OP_PUT + 1;
                return 0;
            }*/


        if (msg->message->opcode == OP_HEIGHT) {
            msg->message->opcode = OP_HEIGHT + 1;
            msg->message->c_type = CT_RESULT;
            pthread_mutex_lock(&tree_lock);
            msg->message->data_size = tree_height(tree);
            pthread_mutex_unlock(&tree_lock);
            return 0;
        }

        if (msg->message->opcode==OP_VERIFY){
            pthread_mutex_lock(&tree_lock);
            if (verify(msg->message->data_size)==0){
                msg->message->opcode=OP_VERIFY+1;
                msg->message->c_type=CT_RESULT;
                msg->message->data_size=1;
                pthread_mutex_unlock(&tree_lock);
                return 0;
            }
            msg->message->opcode=OP_ERROR;
            msg->message->c_type=CT_NONE;
            msg->message->data_size=-1;
            pthread_mutex_unlock(&tree_lock);
            return 0;

        }

        else {
            return -1;
        }
    }


int verify(int op_n){
    if (op_n<=op_count)
    return 0;
    return -1;
}


void *process_task (void *params){
    while (1) {
        pthread_mutex_lock(&queue_lock);

        while (queue_head == NULL) {
            pthread_cond_wait(&queue_not_empty, &queue_lock);
        }
        printf("Há uma nova task!\n");

        struct task_t *task = queue_head;
        queue_head = task->next;
        if ((task->op==1)){
            struct data_t *data = data_create2(sizeof(task->data),task->data);
            pthread_mutex_lock(&tree_lock);
            int output=tree_put(tree, task->key, data);
            if (output==0) {
                op_count+=1;
                //last_assigned=OP_PUT+1;
                //data_destroy(data);
            }

            pthread_mutex_unlock(&tree_lock);
            pthread_mutex_unlock(&queue_lock);
                /*msg->message->c_type = CT_NONE;
                msg->message->opcode = OP_ERROR;*/
            } else {
            pthread_mutex_lock(&tree_lock);
            tree_del(tree, task->key);
                op_count+=1;
                //last_assigned=OP_DEL+1;

            pthread_mutex_unlock(&tree_lock);
            pthread_mutex_unlock(&queue_lock);
            /*msg->message->c_type = CT_NONE;
            msg->message->opcode = OP_PUT + 1;*/
            }
        }
}





void invoke_aux(int listening_socket, struct message_t * message){
    int size=1024;
    repete_put=malloc(size);
    if (ZOK!=zoo_get(zh, root_backup,(void *)watcher_ctx, repete_put, &size, NULL)){
        fprintf(stderr, "Ups\n");
    }
    char *token=strtok(strdup(repete_put),":");
    repete_put=strtok(repete_put,":");
    repete_put=strtok(NULL,":");
    if (atoi(repete_put)!=listening_socket){
        conect_new(repete_put, message, token);
    }

}

int conect_new(char* sockete, struct message_t* message, char*host){



    int len = message_t__get_packed_size(message->message);
    void *buf = malloc(len);
    int size=0;
    message_t__pack(message->message, buf);
    if ((size = write_All(rtree->sock_primary, (char *) &len, sizeof(int))) != sizeof(int)) {
        printf("Erro a enviar dados 1!\n");
        free(buf);
        exit(EXIT_FAILURE);
    }

    if ((size = write_All(rtree->sock_primary, buf, len)) != len) {
        printf("Erro a enviar dados!\n");
        free(buf);
        exit(EXIT_FAILURE);
    }
    return 0;
}

