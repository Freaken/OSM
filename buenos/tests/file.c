#include "tests/lib.h"

int main(void) {
    int filehandle, read;
    char buffer[50];
    buffer[49] = '\0';
    printf("\n\n");

    syscall_create("[disk1]lol", 50);
    filehandle = syscall_open("[disk1]lol");
    syscall_write(filehandle, "LOLOL i trol u!!!", 20);
    syscall_seek(filehandle, 0);
    read = syscall_read(filehandle, buffer, 49);
    syscall_close(filehandle);
    syscall_delete("[disk1]lol");


    printf("buffer: %s, read: %d \n", buffer, read);


    printf("\nYay, you did it!\n\n");

    return 0;
}
