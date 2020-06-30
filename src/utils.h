#ifndef UTILS_H
#define UTILS_H

#include "stdbool.h"

typedef enum Status {
    FAILURE = 0,
    SUCCESS = 1
} Status;

void panic(char *reason, ...);
void panic_at(char *location, char *code, char *reason, ...);
bool has_prefix(char *str, const char *prefix);

#endif // UTILS_H
