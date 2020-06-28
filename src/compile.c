#include "stdio.h"
#include "stdlib.h"
#include "stdarg.h"
#include "ctype.h"

char *global_code;

typedef enum Status {
    FAILURE = 0,
    SUCCESS = 1
} Status;

void panic_at(char *location, char *reason, ...) {
    va_list ap;
    va_start(ap, reason);

    int position = location - global_code;
    fprintf(stderr, "%s\n", global_code);
    fprintf(stderr, "%*s", position, "");
    fprintf(stderr, "^ ");
    vfprintf(stderr, reason, ap);
    fprintf(stderr, "\n");
    exit(1);
}

typedef enum TokenKind {
    TK_RESERVED = 0,
    TK_NUM = 1,
    TK_EOF = 2
} TokenKind;

const char *global_tk_repr[] = {
    "Reserved", "Number", "End Of File"};

typedef struct Token {
    TokenKind kind;
    long int num_val;
    char *str;
    struct Token *next;
} Token;

Token * create_token(TokenKind kind, Token *current_token, char *str) {
    Token *new_token = (Token *)calloc(1, sizeof(Token));
    new_token->kind = kind;
    new_token->str = str;
    current_token->next = new_token;
    return new_token;  // now it is current
}

Token * tokenize(char *p) {
    Token head;
    head.next = NULL;
    Token *current_token = &head;

    while (*p) {
        if (isspace(*p)) {
            ++p;
            continue;
        }

        if (*p == '-' || *p == '+') {
            current_token = create_token(TK_RESERVED, current_token, p);
            ++p;
            continue;
        }

        if (isdigit(*p)) {
            current_token = create_token(TK_NUM, current_token, p);
            current_token->num_val = strtol(p, &p, 10);
            continue;
        }

        panic_at(p, "Lexing error: Can't figure out token.");
    }

    create_token(TK_EOF, current_token, NULL);
    return head.next;
}

Token *global_token_ptr;

Status try_eat_op_token(char op) {
    if (global_token_ptr->kind != TK_RESERVED ||
          *global_token_ptr->str != op) {
        return FAILURE;
    }
    global_token_ptr = global_token_ptr->next;
    return SUCCESS;
}

Token * eat_number_token() {
    if (global_token_ptr->kind != TK_NUM) {
        panic_at(
            global_token_ptr->str,
            "Parsing error: Wrong Token (Got %s, expected %s)",
            global_tk_repr[global_token_ptr->kind], global_tk_repr[TK_NUM]
        );
    }
    Token *num_token = global_token_ptr;
    global_token_ptr = global_token_ptr->next;
    return num_token;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Not enough arguments\n");
        return 1;
    }

    global_code = argv[1];
    global_token_ptr = tokenize(global_code);

    printf(".intel_syntax_noprefix\n");
    printf(".globl _main\n");
    printf("_main:\n");

    printf(" mov rax, %ld\n", eat_number_token()->num_val);

    while (global_token_ptr->kind != TK_EOF) {
        if (try_eat_op_token('+')) {
            printf(" add rax, %ld\n", eat_number_token()->num_val);
            continue;
        }

        if (try_eat_op_token('-')) {
            printf(" sub rax, %ld\n", eat_number_token()->num_val);
            continue;
        }

        panic_at(global_token_ptr->str, "Parsing error: Wrong token");
    }

    printf(" ret\n");
    return 0;
}
