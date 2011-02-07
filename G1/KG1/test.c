#include <stdlib.h>
#include <stdio.h>
#include "bintree.h"
#include "dlist.h"

// Function that compares ints for use with insert2
int compare(void *data1, void *data2) {
    int *int1 = data1;
    int *int2 = data2;
    if(*int1 < *int2)
        return -1;
    if(*int1 == *int2)
        return 0;
    if(*int1 > *int2)
        return 1;

    printf("LOL");
    exit(1);
}

// Visualizes binary search trees created with insert
void print_node(tnode_t *node) {
    printf("adress: %p value: %d lchild: %p rchild: %p \n", (void *) node, node->data,(void *) node->lchild, (void *) node->rchild);

    if(node->lchild != NULL) {
        print_node(node->lchild);
    }

    if(node->rchild != NULL) {
        print_node(node->rchild);
    }
}


// Visualizes binary search trees created with insert2
void print_node2(tnode_t2 *node) {
    int *value = node->data;
    printf("adress: %p data: %d lchild: %p rchild: %p \n", (void *) node, *value,(void *) node->lchild, (void *) node->rchild);

    if(node->lchild != NULL) {
        print_node2(node->lchild);
    }

    if(node->rchild != NULL) {
        print_node2(node->rchild);
    }
}


int main() {
    // Tests for opgave 1
    tnode_t *tree = NULL;
    insert(&tree, 3);
    insert(&tree, 8);
    insert(&tree, 2);
    insert(&tree, 7);
    insert(&tree, 15);
    insert(&tree, 0);
    insert(&tree, -5);
    insert(&tree, 2);

    print_node(tree);
    printf("The inorder walk for the tree is: \n");
    print_inorder(tree);
    printf("\n");
    printf("The size of the tree is %d\n", size(tree));

    // Tests for opgave 2
    int *array = to_array(tree);

    for (int i = 0; i < size(tree); i++) {
        printf("array[%d] = %d \n", i, array[i]);
    }

    dlist_t *list = tree2dlist(tree);
    for (int i = 0; i < size(tree)+1; i++) {
        printf("element %d i listen er %d\n", i, list->data);
        list = list->next;
    }
    list = list->prev;
    for(int i = 0; i < size(tree)+1; i++) {
        printf("element %d i listen bagfra er %d\n", i, list->data);
        list = list->prev;
    }

    // Tests for opgave 3
    tnode_t2 *tree2 = NULL;
    int tal[20];
    for(int i = 0; i < 20; i++) {
        tal[i] = i;
    }

    insert2(&tree2, &tal[4], &compare);
    insert2(&tree2, &tal[7], &compare);
    insert2(&tree2, &tal[14], &compare);
    insert2(&tree2, &tal[3], &compare);
    insert2(&tree2, &tal[19], &compare);

    print_node2(tree2);

    return 0;
}



