#include "stdio.h"
#include "stdlib.h"
#include "ctype.h"
#include "string.h"

#include "lexer.h"
#include "utils.h"

Token * create_token(TokenKind kind, Token *current_token, char *str, int len) {
    Token *new_token = (Token *)calloc(1, sizeof(Token));
    new_token->kind = kind;
    new_token->str = str;
    new_token->len = len;
    current_token->next = new_token;
    return new_token;
}

Token * tokenize(char *code) {
    char *p = code;
    Token head;
    head.next = NULL;
    head.len = 0;  // C99 does not allow struct init :(
    Token *current_token = &head;
    size_t chars_left = strlen(p);

    while (*p) {
        chars_left -= current_token->len;

        if (isspace(*p)) {
            ++p;
            continue;
        }

        // 2-char ops
        if (chars_left >= 2 && (
                has_prefix(p, "<=") || has_prefix(p, ">=") ||
                has_prefix(p, "==") || has_prefix(p, "!="))) {
            current_token = create_token(TK_RESERVED, current_token, p, 2);
            p += 2;
            continue;
        }

        // 1-char ops
        if (chars_left >= 1 && strchr("()+-*/%<>", *p)) {
            current_token = create_token(TK_RESERVED, current_token, p, 1);
            ++p;
            continue;
        }

        if (isdigit(*p)) {
            current_token = create_token(TK_NUM, current_token, p, 0);
            char *num_begin = p;
            current_token->num_val = strtol(num_begin, &p, 10);
            current_token->len = p - num_begin;
            continue;
        }

        panic_at(p, code, "Lexing error: Can't figure out token.");
    }

    create_token(TK_EOF, current_token, NULL, 0);
    return head.next;
}
