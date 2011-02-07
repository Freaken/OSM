#include "dlist.h"
#include "error.h"
#include <stdlib.h>

dlist_t *list_insert(dlist_t *prev, int data) {
    dlist_t *cur;
    
    (cur = malloc(sizeof(dlist_t))) || FAIL("malloc failed");

    cur->data = data;

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

dlist_t *tree2dlist_helper(tnode_t *tree, dlist_t *prev) {
    if(tree == NULL)
        return prev;

    prev = tree2dlist_helper(tree->lchild, prev);
    prev = list_insert(prev, tree->data);

    return tree2dlist_helper(tree->rchild, prev);
    
}

dlist_t *tree2dlist(tnode_t *tree) {
    return tree2dlist_helper(tree, NULL)->next;
}
