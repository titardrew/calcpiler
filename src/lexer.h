#ifndef LEXER_H
#define LEXER_H

typedef enum TokenKind {
    TK_RESERVED = 0,
    TK_NUM = 1,
    TK_EOF = 2
} TokenKind;

typedef struct Token {
    TokenKind kind;
    int num_val;
    char *str;
    int len;
    struct Token *next;
} Token;

static const char *global_tk_repr[] = {
    "Reserved", "Number", "End Of File"};

Token * tokenize(char *code);

#endif
