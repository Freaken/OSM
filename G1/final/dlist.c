#include "dlist.h"
#include "error.h"
#include <stdlib.h>
#include <stdio.h>

dlist_t *list_insert(dlist_t *prev, int data) {

    /* Allocate memory for the node */
    dlist_t *cur = malloc(sizeof(dlist_t));
    
    if(cur == NULL) {
        printf("malloc failed");
        exit(EXIT_FAILURE);
    }

    /* Inserts data */
    cur->data = data;

    /* Inserts node into current list */
    if(prev == NULL) {
        cur->prev = cur;
        cur->next = cur;
    } else {
        cur->prev = prev;
        cur->next = prev->next;

        prev->next->prev = cur;
        prev->next = cur;
    }

    return cur;
}

void tree2dlist_helper(tnode_t *tree, dlist_t **dlist_pos) {
    /* Return if bottom has been reached */
    if(tree == NULL)
        return;

    /* Dump left side */
    tree2dlist_helper(tree->lchild, dlist_pos);

    /* Dump current */
    *dlist_pos = list_insert(*dlist_pos, tree->data);

    /* Dump right side */
    tree2dlist_helper(tree->rchild, dlist_pos);
    
}

dlist_t *tree2dlist(tnode_t *tree) {
    dlist_t *dlist_pos = NULL;
    
    tree2dlist_helper(tree, &dlist_pos);

    return dlist_pos->next;
}
