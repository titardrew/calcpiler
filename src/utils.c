#include "ctype.h"
#include "stdarg.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "utils.h"

void panic(char *reason, ...) {
    va_list ap;
    va_start(ap, reason);
    vfprintf(stderr, reason, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void panic_at(char *location, char *code, char *reason, ...) {
    va_list ap;
    va_start(ap, reason);

    int position = location - code;
    fprintf(stderr, "%s\n", code);
    fprintf(stderr, "%*s", position, "");
    fprintf(stderr, "^ ");
    vfprintf(stderr, reason, ap);
    fprintf(stderr, "\n");
    exit(1);
}

bool has_prefix(char *str, const char *prefix) {
    return memcmp(prefix, str, strlen(prefix)) == 0;
}
