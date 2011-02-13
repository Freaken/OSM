#include <stdlib.h>
#include <stdio.h>
#include "bintree.h"

void insert(tnode_t **tree, int value) {
    if(*tree == NULL) {

        //Allocate memory for the node
        *tree = malloc(sizeof(tnode_t));
        if(*tree == NULL) {
            printf("malloc failed\n");
            exit(1);
        }

        //Initialize node
        (*tree)->data = value;
        (*tree)->lchild = NULL;
        (*tree)->rchild = NULL;
    }

    else {
        //Traverse the tree to find an empty node at correct place
        if(value <= (*tree)->data) {
            insert(&(*tree)->lchild, value);
        }
        else {
            insert(&(*tree)->rchild, value);
        }
    }
}

void print_inorder_helper(tnode_t* tree) {
    //Stop when we reach the edge
    if(tree == NULL)
        return;

    //Print left part of tree
    print_inorder_helper(tree->lchild);

    //Print value of current node
    printf("%d ", tree->data);

    //Print right part of tree
    print_inorder_helper(tree->rchild);
}

void print_inorder(tnode_t* tree) {
    print_inorder_helper(tree);
    printf("\n");
}

int size(tnode_t *tree) {
    //Tree is empty
    if(tree == NULL) {
        return 0;
    }

    //Recursively find the size
    return 1 + size(tree->lchild) + size(tree->rchild);
}

//Dumps values from the tree to an array
void array_dump(tnode_t *tree, int **arraypos) {
    //Return if bottom has been reached
    if(tree == NULL) {
        return;
    }

    //Left side of tree
    array_dump(tree->lchild, arraypos);

    //Write to array
    **arraypos = tree->data;
    (*arraypos)++;

    //Right side of tree
    array_dump(tree->rchild, arraypos);
}

int* to_array(tnode_t *tree) {
    //Allocate memory for the array
    int *arraypos, *retval;
    arraypos = retval = malloc(sizeof(int)*size(tree));
    if(retval == NULL) {
        printf("malloc failed");
        exit(1);
    }
    //Write to the array
    array_dump(tree, &arraypos);

    return retval;
}

void insert2(tnode_t2 **tree, void *data, int (*comp)(void *, void *)) {
    if(*tree == NULL) {

        //Allocate memory for the node
        *tree = malloc(sizeof(tnode_t2));
        if(*tree == NULL) {
            printf("malloc failed\n");
            exit(1);
        }

        //Initialize node
        (*tree)->data = data;
        (*tree)->lchild = NULL;
        (*tree)->rchild = NULL;
    }

    else {
        //Traverse the tree to find an empty node at correct place
        if((*comp)(data, (*tree)->data) < 1) {
            insert2(&(*tree)->lchild, data, (*comp));
        }
        else {
            insert2(&(*tree)->rchild, data, (*comp));
        }
    }

}
