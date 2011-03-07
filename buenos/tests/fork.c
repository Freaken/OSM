#include "tests/lib.h"

void new_funct(int arg) {
    int n;

    arg = arg;

    for(n = 0; n < 700; n++) {
//        printf("The arg was %d\n", arg);
        putc('a');
    }

    syscall_exit(17);
}

int main(void) {
    int n;

    syscall_fork(new_funct, 17);

    for(n = 0; n < 700; n++) {
//        printf("Main thread\n");
        putc('b');
    }

    return 0;
}
