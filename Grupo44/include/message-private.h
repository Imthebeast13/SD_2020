#ifndef _MESSAGE_PRIVATE_H
#define _MESSAGE_PRIVATE_H
#include <errno.h>
#include "inet.h"
#include "sdmessage.pb-c.h"
#include "client_stub.h"

#define OP_SIZE		10
#define OP_DEL	   	20
#define OP_GET		30
#define OP_PUT		40
#define OP_GETKEYS	50
#define OP_HEIGHT   60
#define OP_VERIFY   70

#define OP_ERROR	99

#define CT_KEY    10
#define CT_VALUE  20
#define CT_ENTRY  30
#define CT_KEYS   40
#define CT_RESULT 50
#define CT_NONE   60
    struct message_t{

    MessageT *message;
    /*short opCode;
    short cType;
    union content{
        char*key;
        struct data_t* value;
        struct entry_t* entry;
        char**keys;
        int result;*/


};



int write_All(int sock,char *buf,int len);

int read_All(int socket,char *buf,int len);

//void destroy_message(struct message_t *message);

#endif
