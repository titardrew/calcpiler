#include "stdio.h"
#include "stdlib.h"

#include "gen.h"
#include "utils.h"

void gen_ast_node_code(AST_Node *node) {
    if (node->kind == AST_NUM) {
        printf(" push %d\n", node->num_val);
        return;
    }

    gen_ast_node_code(node->left);
    gen_ast_node_code(node->right);

    printf(" pop rdi\n");
    printf(" pop rax\n");
    
    switch (node->kind) {
      case AST_ADD:
        printf(" add rax, rdi\n");
        break;
      case AST_SUB:
        printf(" sub rax, rdi\n");
        break;
      case AST_MUL:
        printf(" imul rax, rdi\n");
        break;
      case AST_DIV:
        // cqo extends rax to rdx:rax, filling rdx bits with rax sign
        printf(" cqo\n");
        // now, idiv performs signed division: rax:=rax/rdi, rdx:=rax%rdi
        printf(" idiv rdi\n");
        break;
      case AST_MOD:
        printf(" cqo\n");
        printf(" idiv rdi\n");
        printf(" mov rax, rdx\n");
        break;
      case AST_EQ:
        printf(" cmp rdi, rax\n");
        printf(" sete al\n"); // al - first 8 bits of rax
        printf(" movzx rax, al\n"); // movzx->movzb for linux
        break;
      case AST_NEQ:
        printf(" cmp rdi, rax\n");
        printf(" setne al\n");
        printf(" movzx rax, al\n");
        break;
      case AST_GR:
        printf(" cmp rdi, rax\n");
        printf(" setg al\n");
        printf(" movzx rax, al\n");
        break;
      case AST_GE:
        printf(" cmp rdi, rax\n");
        printf(" setge al\n");
        printf(" movzx rax, al\n");
        break;
      default:
        panic("Internal error in gen_ast_node_code(AST_Node *)");
    }
    printf(" push rax\n");
}

void gen_code(AST_Node *ast_head) {
    printf(".intel_syntax_noprefix\n");
    printf(".globl _main\n");
    printf("_main:\n");
    gen_ast_node_code(ast_head);
    printf(" pop rax\n");
    printf(" ret\n");
}
