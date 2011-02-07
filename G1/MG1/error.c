#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

void *error(char *msg, char *file, int line) {
    fprintf(stderr, "%s, at %s:%d:\n\nError code indicates: %s\n", msg, file, line, strerror(errno));
    exit(-1);
}
