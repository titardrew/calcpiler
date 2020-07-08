#ifndef LEXER_H
#define LEXER_H

typedef enum TokenKind {
    TK_RESERVED = 0,
    TK_IDENT    = 1,
    TK_NUM      = 2,
    TK_RET      = 3,
    TK_IF       = 4,
    TK_ELSE     = 5,
    TK_WHILE    = 6,
    TK_FOR      = 7,
    TK_EOF      = 8
} TokenKind;

typedef struct Token {
    TokenKind kind;
    int num_val;
    char *str;
    int len;
    struct Token *next;
} Token;

static const char *global_tk_repr[] = {
    "reserved", "identifier", "number", "return",
    "if", "else", "while", "for", "end of file"};

Token * tokenize(char *code);

#endif // LEXER_H
