#include "bintree.h"
#include <stdio.h>
#include <stdlib.h>

void insert(tnode_t **tree, int data) {
	if (data <= (*tree)->data) {
		if ((*tree)->lchild != NULL) {
			insert(&(*tree)->lchild, data);
		}
		else {
			struct tnode_t *newNode = malloc(sizeof(tnode_t));
			newNode->data = data;
			newNode->lchild = NULL;
			newNode->rchild = NULL;
			(*tree)->lchild = newNode;
		}
	}
	else {
		if ((*tree)->rchild != NULL) {
			insert(&(*tree)->rchild, data);
		}
		else {
			struct tnode_t *newNode = malloc(sizeof(tnode_t));
			newNode->data = data;
			newNode->lchild = NULL;
			newNode->lchild = NULL;
			(*tree)->rchild = newNode;
		}
	}
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
	printf(".\n");
}

int size(tnode_t *tree) {
	if (tree == NULL) {return 0;}
	else {return size(tree->lchild) + size(tree->rchild) + 1;}
}

void to_array_helper(tnode_t *tree, int **dapp) {
	if (tree->lchild != NULL) {
		to_array_helper(tree->lchild, dapp);
	}
	**dapp = tree->data;
	(*dapp)++;
	if (tree->rchild != NULL) {
		to_array_helper(tree->rchild, dapp);
	}
}

int *to_array(tnode_t *tree) {
	int *dataarray = malloc(sizeof(int) * size(tree));
	int **dapp = &dataarray;
	int *result = dataarray;
	to_array_helper(tree, dapp);
	return result;
}
