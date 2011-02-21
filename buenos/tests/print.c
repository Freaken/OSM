#include "tests/lib.h"

int main(void) {
    char test = 0;
    int count = 0;

    printf("\n\n");

    do {
        printf("Write an a: ");
        test = getc();
        printf("\n");
        count++;
    } while(test != 'a');

    printf("\nYay, you did it!\n\n");

    return count;
}
