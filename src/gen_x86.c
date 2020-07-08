#include "stdio.h"
#include "stdlib.h"

#include "gen_x86.h"
#include "utils.h"

static int global_label_counter = 0;

static void push_address(int stack_offset) {
    printf(" mov rax, rbp\n");
    printf(" sub rax, %d\n", stack_offset);
    printf(" push rax\n");
}

static void stack_prologue(int stack_size) {
    printf(" push rbp\n");
    printf(" mov rbp, rsp\n");
    printf(" sub rsp, %d\n", stack_size);
}

static void stack_epilogue() {
    printf(".L.ret:\n");
    printf(" mov rsp, rbp\n");
    printf(" pop rbp\n");
    printf(" ret\n");
}

void gen_ast_node_code(AST_Node *node) {
    int lbl_cnt;
    switch (node->kind) {
      case AST_RET:
        gen_ast_node_code(node->left);
        printf(" pop rax\n");
        printf(" jmp .L.ret\n");
        return;
      case AST_IF:
        lbl_cnt = global_label_counter++;
        gen_ast_node_code(node->cond);
        printf(" pop rax\n");
        printf(" cmp rax, 0\n");
        printf(" je .L.B1_%d\n", lbl_cnt);
        gen_ast_node_code(node->left);
        if (node->right) {
            printf(" jmp .L.B2_%d\n", lbl_cnt);
        }
        printf(".L.B1_%d:\n", lbl_cnt);
        if (node->right) {
            gen_ast_node_code(node->right);
            printf(".L.B2_%d:\n", lbl_cnt);
        }
        return;
      case AST_WHILE:
        lbl_cnt = global_label_counter++;
        printf(".L.B0_%d:\n", lbl_cnt);
        gen_ast_node_code(node->cond);
        printf(" pop rax\n");
        printf(" cmp rax, 0\n");
        printf(" je .L.B1_%d\n", lbl_cnt);
        gen_ast_node_code(node->left);
        printf(" jmp .L.B0_%d\n", lbl_cnt);
        printf(".L.B1_%d:\n", lbl_cnt);
        return;
      case AST_FOR:
        lbl_cnt = global_label_counter++;
        if (node->init) {
            gen_ast_node_code(node->init);
        }
        printf(".L.B0_%d:\n", lbl_cnt);
        if (node->cond) {
            gen_ast_node_code(node->cond);
        }
        printf(" pop rax\n");
        printf(" cmp rax, 0\n");
        printf(" je .L.B1_%d\n", lbl_cnt);
        gen_ast_node_code(node->left);
        if (node->right) {
            gen_ast_node_code(node->right);
        }
        printf(" jmp .L.B0_%d\n", lbl_cnt);
        printf(".L.B1_%d:\n", lbl_cnt);
        return;
      case AST_NUM:
        printf(" push %d\n", node->num_val);
        return;
      case AST_LVALUE:
        push_address(node->stack_offset);
        printf(" pop rax\n");
        printf(" mov rax, [rax]\n");
        printf(" push rax\n");
        return;
      case AST_ASSIGN:
        if (node->left->kind != AST_LVALUE) {
            panic("Generation error: Can only assign to an l-value");
        }
        push_address(node->left->stack_offset);
        gen_ast_node_code(node->right);

        printf(" pop rdi\n");
        printf(" pop rax\n");
        printf(" mov [rax], rdi\n");
        printf(" push rdi\n");
        return;
      default:
        break;
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

void gen_code(AST_Node **ast_forest) {
    printf(".intel_syntax_noprefix\n");
    printf(".globl _main\n");
    printf("_main:\n");

    stack_prologue(8*64); // 64 byte stack
    for (int i = 0; ast_forest[i] != NULL; i++) {
        gen_ast_node_code(ast_forest[i]);
        printf(" pop rax\n");
    }
    stack_epilogue();
}
