#include "stdio.h"
#include "stdlib.h"
#include "stdarg.h"
#include "ctype.h"

char *global_code;

typedef enum Status {
    FAILURE = 0,
    SUCCESS = 1
} Status;

void panic(char *reason, ...) {
    va_list ap;
    va_start(ap, reason);
    vfprintf(stderr, reason, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void panic_at(char *location, char *reason, ...) {
    va_list ap;
    va_start(ap, reason);

    int position = location - global_code;
    fprintf(stderr, "%s\n", global_code);
    fprintf(stderr, "%*s", position, "");
    fprintf(stderr, "^ ");
    vfprintf(stderr, reason, ap);
    fprintf(stderr, "\n");
    exit(1);
}


typedef enum TokenKind {
    TK_RESERVED = 0,
    TK_NUM = 1,
    TK_EOF = 2
} TokenKind;

const char *global_tk_repr[] = {
    "Reserved", "Number", "End Of File"};

typedef struct Token {
    TokenKind kind;
    int num_val;
    char *str;
    struct Token *next;
} Token;

Token * create_token(TokenKind kind, Token *current_token, char *str) {
    Token *new_token = (Token *)calloc(1, sizeof(Token));
    new_token->kind = kind;
    new_token->str = str;
    current_token->next = new_token;
    return new_token;  // now it is current
}

Token * tokenize(char *p) {
    Token head;
    head.next = NULL;
    Token *current_token = &head;

    while (*p) {
        if (isspace(*p)) {
            ++p;
            continue;
        }

        if (*p == '-' || *p == '+' || *p == '*' || *p == '/' ||
                *p == '(' || *p == ')') {
            current_token = create_token(TK_RESERVED, current_token, p);
            ++p;
            continue;
        }

        if (isdigit(*p)) {
            current_token = create_token(TK_NUM, current_token, p);
            current_token->num_val = strtol(p, &p, 10);
            continue;
        }

        panic_at(p, "Lexing error: Can't figure out token.");
    }

    create_token(TK_EOF, current_token, NULL);
    return head.next;
}

typedef enum AST_NodeKind {
    AST_ADD = 0,
    AST_SUB = 1,
    AST_MUL = 2,
    AST_DIV = 3,
    AST_NUM = 4
} AST_NodeKind;

typedef struct AST_Node{
    AST_NodeKind kind;
    int num_val;
    struct AST_Node *left;
    struct AST_Node *right;
} AST_Node;

AST_Node * create_ast_op_node(AST_NodeKind kind,
        AST_Node *left, AST_Node *right) {
    AST_Node *node = (AST_Node *)calloc(1, sizeof(AST_Node));
    node->kind = kind;
    node->left = left;
    node->right = right;
    return node;
}

AST_Node * create_ast_num_node(int num_val) {
    AST_Node *node = (AST_Node *)calloc(1, sizeof(AST_Node));
    node->kind = AST_NUM;
    node->num_val = num_val;
    return node;
}

Token *global_token_ptr;

Status try_eat_op_token(char op) {
    if (global_token_ptr->kind != TK_RESERVED ||
          *global_token_ptr->str != op) {
        return FAILURE;
    }
    global_token_ptr = global_token_ptr->next;
    return SUCCESS;
}

Token * eat_number_token() {
    if (global_token_ptr->kind != TK_NUM) {
        panic_at(
            global_token_ptr->str,
            "Parsing error: Wrong Token (Got %s, expected %s)",
            global_tk_repr[global_token_ptr->kind], global_tk_repr[TK_NUM]
        );
    }
    Token *num_token = global_token_ptr;
    global_token_ptr = global_token_ptr->next;
    return num_token;
}

AST_Node * parse_expression();
AST_Node * parse_mul();
AST_Node * parse_atom();

AST_Node * parse_expression() {
    AST_Node *node = parse_mul();

    while (1) {
        if (try_eat_op_token('+')) {
            node = create_ast_op_node(AST_ADD, node, parse_mul());
        } else if (try_eat_op_token('-')) {
            node = create_ast_op_node(AST_SUB, node, parse_mul());
        } else {
            return node;
        }
    }
}

AST_Node * parse_mul() {
    AST_Node *node = parse_atom();

    while (1) {
        if (try_eat_op_token('*')) {
            node = create_ast_op_node(AST_MUL, node, parse_atom());
        } else if (try_eat_op_token('/')) {
            node = create_ast_op_node(AST_DIV, node, parse_atom());
        } else {
            return node;
        }
    }
}

AST_Node * parse_atom() {
    if (try_eat_op_token('(')) {
        AST_Node *node = parse_expression();
        if (try_eat_op_token(')')) {
            return node;
        } else {
            panic_at(global_token_ptr->str,
                     "Parsing error: Wrong token [Expected ')']");
        }
    }
    return create_ast_num_node(eat_number_token()->num_val);
}

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
        printf(" cqo\n");
        printf(" idiv rdi\n");
        break;
      default:
        panic("Internall error in gen_ast_node_code(AST_Node *)");
    }
    printf(" push rax\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Not enough arguments\n");
        return 1;
    }

    global_code = argv[1];
    global_token_ptr = tokenize(global_code);
    AST_Node *ast_head = parse_expression();

    printf(".intel_syntax_noprefix\n");
    printf(".globl _main\n");
    printf("_main:\n");

    gen_ast_node_code(ast_head);

    printf(" pop rax\n");
    printf(" ret\n");
    return 0;
}
