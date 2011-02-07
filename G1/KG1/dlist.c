#include <stdlib.h>
#include <stdio.h>
#include "dlist.h"
#include "bintree.h"

void list_dump(tnode_t *tree, dlist_t *list, int *count, int length) {
       if(tree == NULL) {
        return;
    }

    //Left side of tree
    list_dump(tree->lchild, list, count, length);

    //Write to array
    (list + *count)->data = tree->data;
    (list + *count)->prev = (list + *count - 1);

    //Check for last object 
    if(*count+1 < length) {
        (list + *count)->next = (list + *count + 1);
    }
    else {
        (list + *count)->next = list;
        list->prev = (list + *count);
    }
    (*count)++;

    //Right side of tree
    list_dump(tree->rchild, list, count, length);


}


dlist_t* tree2dlist(tnode_t *tree) {
    int length = size(tree);
    dlist_t *list = malloc(sizeof(dlist_t)*length);
    if(list == NULL) {
        printf("malloc failed");
        exit(1);
    }

    int count = 0;

    list_dump(tree, list, &count, length);
    return list;
}
