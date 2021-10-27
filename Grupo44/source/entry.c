#include "data.h"
#include "entry.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <assert.h>
/*Grupo 44
Nuno Miguel Estalagem 52828
André Firmino 44999
João Janeiro 52779*/
struct entry_t *entry_create(char *key, struct data_t *data){
    struct entry_t* entryT;
    entryT= (struct entry_t*)malloc(sizeof(struct entry_t));
    //entryT->key=malloc(sizeof(char)*sizeof(key));
    //entryT->value=malloc(sizeof(struct data_t*));
    entryT->key= key;
    entryT->value= data;
    return entryT;
    }



void entry_initialize(struct entry_t *entry){
     entry_create(NULL,NULL);

}

void entry_destroy(struct entry_t *entry){
    if(entry!=NULL){
    data_destroy(entry->value);
    free(entry->key);
    free(entry);
    }
    

}
struct entry_t *entry_dup(struct entry_t *entry){
    if(entry==NULL || entry->key==NULL || entry->value ==NULL) return NULL;

    struct data_t* duplicado = data_dup(entry->value);
    char*chave=malloc(sizeof(entry->key));
    strcpy(chave,entry->key);
    return entry_create(chave,duplicado);




}

void entry_replace(struct entry_t *entry, char *new_key, struct data_t *new_value){
    if(entry!=NULL) {
    free(entry->key);
    data_destroy(entry->value);


    entry->value=new_value;
    entry->key=new_key;
    }
   
}

int entry_compare(struct entry_t *entry1, struct entry_t *entry2){
    int a =strcmp(entry1->key,entry2->key);
    if(a==0){
        return 0;
    }
    if(a<0){
        return -1;
    }
    return 1;

}
