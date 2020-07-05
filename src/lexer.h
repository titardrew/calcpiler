#ifndef LEXER_H
#define LEXER_H

typedef enum TokenKind {
    TK_RESERVED = 0,
    TK_IDENT    = 1,
    TK_NUM      = 2,
    TK_EOF      = 3
} TokenKind;

typedef struct Token {
    TokenKind kind;
    int num_val;
    char *str;
    int len;
    struct Token *next;
} Token;

static const char *global_tk_repr[] = {
    "Reserved", "Identifier", "Number", "End Of File"};

Token * tokenize(char *code);

#endif // LEXER_H
