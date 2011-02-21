#include "tests/lib.h"

int main(void) {
    char msg1[] = "\n\nWrite an a!\n\n";
    char msg2[] = "\n\nYay, you did it!\n\n";
    char test = 0;


    do {
        syscall_write(stdout, msg1, sizeof(msg1)-1);
        syscall_read(stdin, &test, 1);
    } while(test != 'a');

    syscall_write(stdout, msg2, sizeof(msg2)-1);

    syscall_halt();

    return 0;
}
