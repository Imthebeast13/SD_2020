#ifndef _TREE_PRIVATE_H
#define _TREE_PRIVATE_H

#include "tree.h"

/*Grupo 44
#Nuno Miguel Estalagem 52828
#André Firmino 44999
#João Janeiro 52779*/

struct tree_t {
	/** a preencher pelo grupo */
	struct entry_t * root;
	struct tree_t * left, *right;
};

// struct node_t{
// 	struct entry_t * node;
// };
struct tree_t * tree_dup(struct tree_t* node);

char** colocaArray(char** lista, int*pointer, int tamanho,struct tree_t* tree);

struct tree_t* tree_del_aux(struct tree_t* tree, char* key);
struct tree_t* minValue(struct tree_t* tree);

#endif