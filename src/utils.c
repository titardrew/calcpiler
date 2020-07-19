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

    char *line_start = location;
    while (line_start != code && *line_start != '\n') line_start--;
    line_start++;
    char *line_end = location;
    while (*line_end != '\0' && *line_end != '\n') line_end++;
    line_end--;

    int position = location - line_start;
    fprintf(stderr, "%.*s\n", (int)(line_end - line_start), line_start);
    fprintf(stderr, "%*s", position, "");
    fprintf(stderr, "\033[0;31m^ ");
    vfprintf(stderr, reason, ap);
    fprintf(stderr, "\033[0;m\n");
    exit(1);
}

bool has_prefix(char *str, const char *prefix) {
    return memcmp(prefix, str, strlen(prefix)) == 0;
}
