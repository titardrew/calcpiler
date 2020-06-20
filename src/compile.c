#include "stdio.h"
#include "stdlib.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Not enough arguments\n");
        return 1;
    }

    printf(".intel_syntax noprefix\n");
    printf(".globl _main\n");
    printf("_main:\n");
    printf("  mov rax, %s\n", argv[1]);
    printf("  ret");

    return 0;
}
