#include "bintree.h"
#include "error.h"
#include <stdlib.h>
#include <stdio.h>

tnode_t *make_tree(int data) {
    tnode_t *tree;
    
    (tree = malloc(sizeof(tnode_t))) || FAIL("malloc failed");

    tree->lchild = NULL;
    tree->rchild = NULL;
    tree->data = data;

    return tree;
}

tnode_t2 *make_tree2(void *data) {
    tnode_t2 *tree;
    
    (tree = malloc(sizeof(tnode_t2))) || FAIL("malloc failed");

    tree->lchild = NULL;
    tree->rchild = NULL;
    tree->data = data;

    return tree;
}

void insert(tnode_t **tree, int data) {
    tnode_t **cur_tree = tree;

    while(*cur_tree != NULL)
        cur_tree = data <= (*cur_tree)->data ?
                   &(*cur_tree)->lchild :
                   &(*cur_tree)->rchild;

    *cur_tree = make_tree(data);
}

void insert2(tnode_t2 **tree, void *data, int (*comp)(void*, void*)) {
    tnode_t2 **cur_tree = tree;

    while(*cur_tree != NULL)
        cur_tree = (*comp)(data, (*cur_tree)->data) != 1 ?
                   &(*cur_tree)->lchild :
                   &(*cur_tree)->rchild;

    *cur_tree = make_tree2(data);
}

void print_inorder_helper(tnode_t *tree) {
    if(tree->lchild != NULL) {
        print_inorder_helper(tree->lchild);
        printf(", ");
    }

    printf("%d", tree->data);

    if(tree->rchild != NULL) {
        printf(", ");
        print_inorder_helper(tree->rchild);
    }
}

void print_inorder(tnode_t *tree) {
    print_inorder_helper(tree);
    printf("\n");
}

int size(tnode_t *tree) {
    if(tree == NULL)
        return 0;

    return size(tree->lchild) + 1 + size(tree->rchild);
}

int *to_array_helper(tnode_t *tree, int *addr) {
    if(tree == NULL)
        return addr;

    addr = to_array_helper(tree->lchild, addr);
    *addr = tree->data;

    return to_array_helper(tree->rchild, addr + 1);
}

int *to_array(tnode_t *tree) {
    int *array;
    
    (array = malloc(sizeof(tnode_t)*size(tree))) || FAIL("malloc failed");

    to_array_helper(tree, array);

    return array;
}
