#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "gen_x86.h"
#include "utils.h"

#define TOO_LONG_FUNC_NAME 512

static int global_label_counter = 0;
static char global_funcname[TOO_LONG_FUNC_NAME];
static char *call_reg8[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

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
    printf(".L.ret.%s:\n", global_funcname);
    printf(" mov rsp, rbp\n");
    printf(" pop rbp\n");
    printf(" ret\n");
}

void gen_ast_node_code(AST_Node *node) {
    int lbl_cnt;
    switch (node->kind) {
      case AST_RET:
        lbl_cnt = global_label_counter++;
        gen_ast_node_code(node->left);
        printf(" pop rax\n");
        printf(" jmp .L.ret.%s\n", global_funcname);
        printf(" push rax\n");
        return;
      case AST_FCALL:
        lbl_cnt = global_label_counter++;
        if (node->args) {
            if (node->args->size > 6) {
                panic("Function %.*s has more than 6 arguments!\n",
                      node->func->len, node->func->name);
            }
            for (int i_arg = 0; i_arg < node->args->size; i_arg++) {
                gen_ast_node_code(&node->args->data[i_arg]);
            }
            for (int i_arg = node->args->size - 1; i_arg >= 0; i_arg--) {
                printf(" pop %s\n", call_reg8[i_arg]); 
            }
        }
        // TODO: evaluate arguments + push them
        // ABI requires rax to be 16 aligned.
        printf(" mov rax, rsp\n");
        printf(" and rax, 15\n");
        printf(" jnz .L.call.%d\n", lbl_cnt);
        printf(" mov rax, 0\n");
        printf(" call %.*s\n", node->func->len, node->func->name);
        printf(" jmp .L.end.%d\n", lbl_cnt);
        printf(".L.call.%d:\n", lbl_cnt);
        printf(" sub rsp, 8\n");
        printf(" mov rax, 0\n");
        printf(" call %.*s\n", node->func->len, node->func->name);
        printf(" add rsp, 8\n");
        printf(".L.end.%d:\n", lbl_cnt);
        printf(" push rax\n");
        return;
      case AST_FUNC:
      case AST_BLOCK:
        for (int i = 0; i < node->block->size; ++i) {
            gen_ast_node_code(&node->block->data[i]);
            printf(" pop rax\n");
        }
        printf(" push rax\n");
        return;
      case AST_IF:
        lbl_cnt = global_label_counter++;
        gen_ast_node_code(node->cond);
        printf(" pop rax\n");
        printf(" cmp rax, 0\n");
        printf(" je .L.B1_%d\n", lbl_cnt);
        gen_ast_node_code(node->left);
        printf(" pop rax\n");
        if (node->right) {
            printf(" jmp .L.B2_%d\n", lbl_cnt);
        }
        printf(".L.B1_%d:\n", lbl_cnt);
        if (node->right) {
            gen_ast_node_code(node->right);
            printf(" pop rax\n");
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
        printf(" pop rax\n");
        printf(" jmp .L.B0_%d\n", lbl_cnt);
        printf(".L.B1_%d:\n", lbl_cnt);
        return;
      case AST_FOR:
        lbl_cnt = global_label_counter++;
        if (node->init) {
            gen_ast_node_code(node->init);
            printf(" pop rax\n");
        }
        printf(".L.B0_%d:\n", lbl_cnt);
        if (node->cond) {
            gen_ast_node_code(node->cond);
            printf(" pop rax\n");
            printf(" cmp rax, 0\n");
            printf(" je .L.B1_%d\n", lbl_cnt);
        }
        gen_ast_node_code(node->left);
        printf(" pop rax\n");
        if (node->right) {
            gen_ast_node_code(node->right);
            printf(" pop rax\n");
        }
        printf(" jmp .L.B0_%d\n", lbl_cnt);
        printf(".L.B1_%d:\n", lbl_cnt);
        printf(" push rax\n");
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
    for (int i = 0; ast_forest[i]; i++) {
        Function *func = ast_forest[i]->func; 
        AST_Node *node = ast_forest[i]; 
        if (func->len == 4 && !memcmp(func->name, "main", 4)) {
            func->is_entry = true;
            printf("\n.globl _main\n");
            printf("_main:\n");
        } else {
            printf("\n.globl %.*s\n", func->len, func->name);
            printf("%.*s:\n", func->len, func->name);
        }
        if (func->len >= TOO_LONG_FUNC_NAME) {
            panic("Too long func name: %.*s", func->len, func->name);
        }
        snprintf(global_funcname, func->len+1, "%.*s",
                 func->len, func->name);
        stack_prologue(8*64); // 64 byte stack
        if (node->args) {
            for (int i_arg = 0; i_arg < node->args->size; i_arg++) {
                printf(" mov [rbp-%d], %s\n",
                       node->args->data[i_arg].decl->stack_offset,
                       call_reg8[i_arg]);
            }
        }
        gen_ast_node_code(node);
        printf(" pop rax\n");
        stack_epilogue();
    }
}
