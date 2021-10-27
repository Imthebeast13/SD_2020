#ifndef REMOTE_TREE_PRIVATE_H
#define REMOTE_TREE_PRIVATE_H
#include "inet.h"
#include "client_stub.h"


struct rtree_t{
    struct sockaddr_in endereco_primary;
    int sock_primary;
    struct sockaddr_in endereco_backup;
    int sock_backup;

};

#endif
