#include "tests/lib.h"

int main(void) {

    char file[] = "[disk1]fork";

    printf("Press a key to spawn %s\n", file);

    getc_raw();

    int pid = syscall_exec(file);
    int retval = syscall_join(pid);

    printf("%s exited with error-code %d\n", file, retval);

    syscall_halt();

    return 0;
}
