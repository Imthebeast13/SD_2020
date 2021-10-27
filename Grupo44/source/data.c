#include "data.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
/*Grupo 44
Nuno Miguel Estalagem 52828
André Firmino 44999
João Janeiro 52779*/
struct data_t *data_create(int size){
    
    if (size<0)
        {
            return NULL;
        }

    struct data_t* tipo_dados;
    tipo_dados=(struct data_t*) malloc(sizeof(struct data_t));
    tipo_dados->datasize=size;
    tipo_dados->data=malloc(size);
    return tipo_dados;
}

struct data_t *data_create2(int size, void *data){
    
    if (size<0)
        {
            return NULL;
        }
    struct data_t* tipo_dados;
    tipo_dados=(struct data_t*) malloc(sizeof(struct data_t));
    tipo_dados->datasize=size;
    tipo_dados->data=data;

    return tipo_dados;


}

void data_destroy(struct data_t *data){
    if (data!=NULL){
        if(data->data!=NULL){
            free(data->data);
        }
    free(data);
    }
  
}

struct data_t *data_dup(struct data_t *data){
    if(data==NULL || data->datasize<=0){
        return NULL;
        }
    if(data->data!=NULL){
        void * copia = malloc(data->datasize);
        memcpy(copia,data->data,data->datasize);
        return data_create2(data->datasize,copia);
    }
    return NULL;
}






void data_replace(struct data_t *data, int new_size, void *new_data){
    if(data!=NULL){
  free(data->data);
  //data->data=malloc(new_size);
  data->data=new_data;
  data->datasize=new_size;
    }
}






