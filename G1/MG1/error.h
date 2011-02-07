#ifndef __ERROR_H
#define __ERROR_H

#define FAIL(msg) error(msg, __FILE__, __LINE__)

void *error(char *msg, char *file, int line);

#endif
