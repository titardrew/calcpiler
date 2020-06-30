#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "gen.h"
#include "parser.h"
#include "utils.h"

Token *global_token_ptr;
char  *global_code;

static AST_Node * create_ast_op_node(AST_NodeKind kind,
        AST_Node *left, AST_Node *right) {
    AST_Node *node = (AST_Node *)calloc(1, sizeof(AST_Node));
    node->kind = kind;
    node->left = left;
    node->right = right;
    return node;
}

static AST_Node * create_ast_num_node(int num_val) {
    AST_Node *node = (AST_Node *)calloc(1, sizeof(AST_Node));
    node->kind = AST_NUM;
    node->num_val = num_val;
    return node;
}

static Status try_eat_op_token(char *op) {
    if (global_token_ptr->kind != TK_RESERVED ||
          strlen(op) != global_token_ptr->len ||
          memcmp(global_token_ptr->str, op, global_token_ptr->len)) {
        return FAILURE;
    }
    global_token_ptr = global_token_ptr->next;
    return SUCCESS;
}

static Token * eat_number_token() {
    if (global_token_ptr->kind != TK_NUM) {
        panic_at(
            global_token_ptr->str, global_code,
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

static AST_Node * parse_expression();
static AST_Node * parse_equality();
static AST_Node * parse_comparison();
static AST_Node * parse_add();
static AST_Node * parse_mul();
static AST_Node * parse_unary();
static AST_Node * parse_atom();

// expression := equality
static AST_Node * parse_expression() {  // to match the grammar
    return parse_equality();
}

// equality := comparison ("==" comparison | "!=" comparison)* 
static AST_Node * parse_equality() {
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

// comparison := add ("<" add | ">" add | "<=" add | ">=" add)* 
static AST_Node * parse_comparison() {
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

// add := mul ("+" mul | "-" mul)*
static AST_Node * parse_add() {
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

// mul := unary ("*" unary | "/" unary | "%" unary)*
static AST_Node * parse_mul() {
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

// unary := ("+" | "-")? atom
static AST_Node * parse_unary() {
    if (try_eat_op_token("+")) {
        return parse_atom();  // noop
    }
    if (try_eat_op_token("-")) {
        return create_ast_op_node(
            AST_SUB, create_ast_num_node(0), parse_atom());
    }
    return parse_atom();
}

// atom := num | "(" expression ")"
static AST_Node * parse_atom() {
    if (try_eat_op_token("(")) {
        AST_Node *node = parse_expression();
        if (try_eat_op_token(")")) {
            return node;
        } else {
            panic_at(
                global_token_ptr->str, global_code,
                "Parsing error: Wrong token [Expected ')']"
            );
        }
    }
    return create_ast_num_node(eat_number_token()->num_val);
}

AST_Node * parse_code(char *code, Token *tokens) {
    global_code = code;
    global_token_ptr = tokens;
    return parse_expression();
}
