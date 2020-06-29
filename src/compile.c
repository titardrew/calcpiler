#include "stdio.h"
#include "stdlib.h"
#include "stdarg.h"
#include "ctype.h"
#include "stdbool.h"
#include "string.h"

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

bool has_prefix(char *str, const char *prefix) {
    return memcmp(prefix, str, strlen(prefix)) == 0;
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
    int len;
    struct Token *next;
} Token;

Token * create_token(TokenKind kind, Token *current_token, char *str, int len) {
    Token *new_token = (Token *)calloc(1, sizeof(Token));
    new_token->kind = kind;
    new_token->str = str;
    new_token->len = len;
    current_token->next = new_token;
    return new_token;
}

Token * tokenize(char *p) {
    Token head;
    head.next = NULL;
    head.len = 0;  // C99 does not allow struct init :(
    Token *current_token = &head;
    size_t chars_left = strlen(p);

    while (*p) {
        chars_left -= current_token->len;

        if (isspace(*p)) {
            ++p;
            continue;
        }

        // 2-char ops
        if (chars_left >= 2 && (
                has_prefix(p, "<=") || has_prefix(p, ">=") ||
                has_prefix(p, "==") || has_prefix(p, "!="))) {
            current_token = create_token(TK_RESERVED, current_token, p, 2);
            p += 2;
            continue;
        }

        // 1-char ops
        if (chars_left >= 1 && strchr("()+-*/%<>", *p)) {
            current_token = create_token(TK_RESERVED, current_token, p, 1);
            ++p;
            continue;
        }

        if (isdigit(*p)) {
            current_token = create_token(TK_NUM, current_token, p, 0);
            char *num_begin = p;
            current_token->num_val = strtol(num_begin, &p, 10);
            current_token->len = p - num_begin;
            continue;
        }

        panic_at(p, "Lexing error: Can't figure out token.");
    }

    create_token(TK_EOF, current_token, NULL, 0);
    return head.next;
}

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

Status try_eat_op_token(char* op) {
    if (global_token_ptr->kind != TK_RESERVED ||
          strlen(op) != global_token_ptr->len ||
          memcmp(global_token_ptr->str, op, global_token_ptr->len)) {
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


/* Grammar (EBNF):
     expression := equality
     equality   := comparison ("==" comparison | "!=" comparison)*
     comparison := add ("<" add | ">" add | "<=" add | ">=" add)*
     add        := mul ("+" mul | "-" mul)*
     mul        := unary ("*" unary | "/" unary | "%" unary)*
     unary      := ("+" | "-")? atom
     atom       := num | "(" expression ")"
*/ 

AST_Node * parse_expression();
AST_Node * parse_equality();
AST_Node * parse_comparison();
AST_Node * parse_add();
AST_Node * parse_mul();
AST_Node * parse_unary();
AST_Node * parse_atom();

AST_Node * parse_expression() {  // to match the grammar
    return parse_equality();
}

AST_Node * parse_equality() {
    AST_Node *node = parse_comparison();

    while (1) {
        if (try_eat_op_token("==")) {
            node = create_ast_op_node(AST_EQ, node, parse_comparison());
        } else if (try_eat_op_token("!=")) {
            node = create_ast_op_node(AST_NEQ, node, parse_comparison());
        } else {
            return node;
        }
    }
}

AST_Node * parse_comparison() {
    AST_Node *node = parse_add();

    while (1) {
        if (try_eat_op_token(">")) {
            node = create_ast_op_node(AST_GR, node, parse_add());
        } else if (try_eat_op_token("<=")) {
            node = create_ast_op_node(AST_GE, node, parse_add());
        } else if (try_eat_op_token("<")) {
            node = create_ast_op_node(AST_GR, parse_add(), node);
        } else if (try_eat_op_token(">=")) {
            node = create_ast_op_node(AST_GE, parse_add(), node);
        } else {
            return node;
        }
    }
}

AST_Node * parse_add() {
    AST_Node *node = parse_mul();

    while (1) {
        if (try_eat_op_token("+")) {
            node = create_ast_op_node(AST_ADD, node, parse_mul());
        } else if (try_eat_op_token("-")) {
            node = create_ast_op_node(AST_SUB, node, parse_mul());
        } else {
            return node;
        }
    }
}

AST_Node * parse_mul() {
    AST_Node *node = parse_unary();

    while (1) {
        if (try_eat_op_token("*")) {
            node = create_ast_op_node(AST_MUL, node, parse_unary());
        } else if (try_eat_op_token("/")) {
            node = create_ast_op_node(AST_DIV, node, parse_unary());
        } else if (try_eat_op_token("%")) {
            node = create_ast_op_node(AST_MOD, node, parse_unary());
        } else {
            return node;
        }
    }
}

AST_Node * parse_unary() {
    if (try_eat_op_token("+")) {
        return parse_atom();  // noop
    }
    if (try_eat_op_token("-")) {
        return create_ast_op_node(
            AST_SUB, create_ast_num_node(0), parse_atom());
    }
    return parse_atom();
}

AST_Node * parse_atom() {
    if (try_eat_op_token("(")) {
        AST_Node *node = parse_expression();
        if (try_eat_op_token(")")) {
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
