#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

typedef enum AST_NodeKind {
    AST_ADD = 0,
    AST_SUB = 1,
    AST_MUL = 2,
    AST_DIV = 3,
    AST_MOD = 4,
    AST_GE  = 5, // >=
    AST_GR  = 6, // >
    AST_EQ  = 7, // ==
    AST_NEQ = 8, // !=
    AST_NUM = 5
} AST_NodeKind;

typedef struct AST_Node {
    AST_NodeKind kind;
    int num_val;
    struct AST_Node *left;
    struct AST_Node *right;
} AST_Node;

AST_Node * parse_code(char *code, Token *tokens);

#endif // PARSER_H
