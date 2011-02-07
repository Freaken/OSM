#include "bintree.h"
#include "dlist.h"
#include <stdio.h>
#include <stdlib.h>


int compare1(void* a, void* b) {
    if(a < b) return -1;
    if(a > b) return 1;

    return 0;
}

int compare2(void* a, void* b) {
    return 1;
}

int main(int argc, char **argv) {
    tnode_t2 *tree = NULL;

    insert2(&tree, (void*) 5, &compare2);
    insert2(&tree, (void*) 2, &compare2);
    insert2(&tree, (void*) 7, &compare2);
    insert2(&tree, (void*) 6, &compare2);
    insert2(&tree, (void*) 5, &compare2);

    print_inorder((tnode_t*) tree);

/*    dlist_t *list;
    dlist_t *cur;
    
    list = tree2dlist((tnode_t*) tree);
    cur = list;

    printf("list: ");
    do {
        printf("%d, ", cur->data);
        cur = cur->next;
    } while(cur != list);

    printf("\n");*/
    
    return 0;
}

