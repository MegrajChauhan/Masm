#ifndef _UTILS_
#define _UTILS_

#include <stdio.h>

#define detailed_message(file, line,  msg, ...) fprintf(stderr, "%s: %zu: " msg "\n", file, line, __VA_ARGS__)

#define simple_message(msg, ...) fprintf(stderr, msg "\n", __VA_ARGS__)

#endif
