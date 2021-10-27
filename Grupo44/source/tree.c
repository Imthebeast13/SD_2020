#include "entry.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include "tree-private.h"
/*Grupo 44
Nuno Miguel Estalagem 52828
André Firmino 44999
João Janeiro 52779*/
int counter = 0;


struct tree_t *tree_create() {

    struct tree_t *tree = (struct tree_t *) malloc(sizeof(struct tree_t));
    tree->root = NULL;
    tree->right = NULL;
    tree->left = NULL;
    return tree;

}

void tree_destroy(struct tree_t *tree) {
    if (tree->root != NULL) {
        if (tree->left != NULL)
            tree_destroy(tree->left);
        if (tree->right != NULL) {
            tree_destroy(tree->right);
        }

        entry_destroy(tree->root);

    }
    free(tree);
}


int tree_put(struct tree_t *tree, char *key, struct data_t *value) {

    if (key == NULL || value == NULL || tree == NULL) {
        return -1;
    }
    char *key_copia = strdup(key);
    struct data_t *value_copia = data_dup(value);
    struct entry_t *novaEntrada = entry_create(key_copia, value_copia);


    if (tree->root == NULL) {
        tree->root = entry_dup(novaEntrada);
        entry_destroy(novaEntrada);
        return 0;
    }

    int comp = entry_compare(novaEntrada, tree->root);
    if (comp == 0) {
        tree->root = entry_dup(novaEntrada);
        entry_destroy(novaEntrada);
        return 0;
    } else if (comp == 1) {
        if (tree->right == NULL) {
            tree->right = tree_create();
            tree->right->root = entry_dup(novaEntrada);
            entry_destroy(novaEntrada);
            return 0;
        } else {
            entry_destroy(novaEntrada);
            return tree_put(tree->right, key, value);
        }
    }

    if (tree->left == NULL) {
        tree->left = tree_create();
        tree->left->root = entry_dup(novaEntrada);
        entry_destroy(novaEntrada);
        return 0;
    } else {
        entry_destroy(novaEntrada);
        return tree_put(tree->left, key, value);
    }



}

struct data_t *tree_get(struct tree_t *tree, char *key) {
    if (tree == NULL || tree->root == NULL) {
        return NULL;
    }
    int compare = strcmp(tree->root->key, key);
    if (compare == 0) {
        return data_dup(tree->root->value);
    } else if (compare < 0)
        return tree_get(tree->right, key);
    else if (compare > 0)
        return tree_get(tree->left, key);
    return data_create2(0,NULL);

}

int tree_del(struct tree_t *tree, char *key) {
    if (tree_get(tree, key) == NULL) {
        return -1;
    }
    int size_tree = tree_size(tree);
    tree_del_aux(tree, key);
    if (size_tree == tree_size(tree)) {
        return -1;
    }
    return 0;
}



struct tree_t *tree_dup(struct tree_t *tree) {
    if (tree == NULL) return NULL;
    struct tree_t *current = tree_create();
    current->root = entry_dup(tree->root);
    current->right = tree_dup(tree->right);
    current->left = tree_dup(tree->left);
    return current;

}

struct tree_t *tree_del_aux(struct tree_t *tree, char *key) {
    if (tree == NULL) {
        return NULL;
    }
    int compare = strcmp(tree->root->key, key);
    if (compare < 0) {
        tree->right = tree_del_aux(tree->right, key);
    } else if (compare > 0) {
        tree->left = tree_del_aux(tree->left, key);
    } else {

        if(tree->left==NULL && tree->right==NULL){
            //tree_destroy(tree);
            entry_destroy(tree->root);
            tree->root=NULL;
            return NULL;
        }

        if (tree->left == NULL) {
            struct tree_t *right = tree_dup(tree->right);
            entry_destroy(tree->root);
            tree->root=right->root;
            tree->right=right->right;
            tree->left=right->left;
            return tree;
        }

        if (tree->right == NULL) {
            struct tree_t *left = tree_dup(tree->left);
            entry_destroy(tree->root);
            tree->root=left->root;
            tree->left=left->left;
            tree->right=left->right;
            return tree;
        }


        struct tree_t *node = minValue(tree->right);
        tree->root=entry_dup(node->root);
        tree->right= tree_del_aux(tree->right, node->root->key);

    }
    return tree;

}


struct tree_t *minValue(struct tree_t *tree) {
    struct tree_t *arv=tree_dup(tree);
    while (arv  && arv->left!= NULL) {
        arv = tree->left;
    }
    return arv;
}


int tree_size(struct tree_t *tree) {
    if (tree == NULL) {
        return 0;
    }
    if (tree->root == NULL) {
        return 0;
    } else {
        return (tree_size(tree->right) + 1 + tree_size(tree->left));
    }


}

int tree_height(struct tree_t *tree) {
    if (tree == NULL) return 0;

    int direita = tree_height(tree->right);
    int esquerda = tree_height(tree->left);

    if (esquerda < direita) return (direita + 1);
    else return (esquerda + 1);
}


char **tree_get_keys(struct tree_t *tree) {
    if (tree_size(tree) == 0) return NULL;


    char **lista = (char **) malloc(sizeof(char *) * tree_size(tree) + 1);
    lista[tree_size(tree)] = NULL;


    int a = 0;
    int *ponteiro = &a;
    return colocaArray(lista, ponteiro, tree_size(tree), tree);









}

char **colocaArray(char **lista, int *pointer, int tamanho, struct tree_t *tree) {
    if (tree == NULL || tree->root == NULL) {
        return NULL;
    }
    if (*pointer < tamanho) {
        lista[*pointer] = strdup(tree->root->key);
        (*pointer)++;
        colocaArray(lista, pointer, tamanho, tree->right);
        colocaArray(lista, pointer, tamanho, tree->left);
        return lista;
    }
    return lista;

}

void tree_free_keys(char **keys) {
    int a = 0;
    while (keys[a] != NULL) {
        free(keys[a]);
        a++;
    }


    free(keys);

}



      

