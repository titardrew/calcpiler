#ifndef GEN_H
#define GEN_H

#include "parser.h"

void gen_ast_node_code(AST_Node *node);
void gen_code(AST_Node *ast_head);

#endif // GEN_H
