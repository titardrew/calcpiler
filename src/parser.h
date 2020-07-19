#ifndef PARSER_H
#define PARSER_H

#include "stdlib.h"

#include "lexer.h"
#include "utils.h"

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
    AST_NUM = 9,

    AST_ASSIGN = 10, // =
    AST_LVALUE = 11,
    AST_RET    = 12, // return
    AST_IF     = 13, // if-else
    AST_WHILE  = 14, // while loop
    AST_FOR    = 15, // for loop
    AST_BLOCK  = 16, // {...}
    AST_FCALL  = 17, // foo()
    AST_FUNC   = 18  // foo(){...}
} AST_NodeKind;

static char *global_node_kind[] = {
    "ADD", "SUB", "MUL", "DIV",
    "MOD", "GE", "GR", "EQ", "NEQ",
    "NUM", "ASSIGN", "LVALUE", "RET",
    "IF", "WHILE", "FOR", "BLOCK",
    "FCALL", "FUNC"};

typedef struct AST_NodeVec AST_NodeVec;
typedef struct Function Function;
typedef struct Declared Declared;
typedef struct AST_Node AST_Node;
typedef struct Scope Scope;

struct Function {
    char *name;
    int len;
    int n_args;
    bool is_entry;
    AST_Node *node;
    Function *next;
};

struct Declared {
    char *name;
    int   len;
    int   stack_offset;
    bool  is_defined;
    Declared *next;
};

struct Scope {
    Declared *locals;
    Scope    *higher_scope;
};

struct AST_Node {
    AST_NodeKind kind;
    int num_val;
    int stack_offset;
    bool has_undef_children;

    Declared    *decl;
    Function    *func;

    AST_Node    *left;
    AST_Node    *right;
    AST_Node    *cond;
    AST_Node    *init;

    AST_NodeVec *block;
    AST_NodeVec *args;
};

struct AST_NodeVec {
    AST_Node *data;
    size_t capacity;
    size_t size;
};

#define AST_NODEVEC_CAPACITY 20

void ast_nodevec_alloc(AST_NodeVec *vec, size_t capacity);
void ast_nodevec_reserve(AST_NodeVec *vec, size_t capacity);
void ast_nodevec_append(AST_NodeVec *vec, AST_Node *node);
AST_Node ** parse_unit(char *code, Token *tokens);

#endif // PARSER_H
