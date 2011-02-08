#include <stdio.h>
#include "bintree.h"
#include "bintree.c"

main() {
	struct tnode_t *tree = malloc(sizeof(tnode_t));
	tree->data = 5;
	tree->lchild = NULL;
	tree->rchild = NULL;

	printf("%d\n", size(tree));

	insert(&tree, 3);
	print_inorder(tree);
	printf("%d\n", size(tree));

	insert(&tree, 3);
	print_inorder(tree);
	printf("%d\n", size(tree));

	int *array = to_array(tree);

	int i = 0;
	while (i < size(tree)) {
		printf("%d ", *array);
		i++;
		array++;
	}
	printf("\n");
}
