#include "stdio.h"
#include "stdlib.h"
#include "string.h"

//#include "gen.h"
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

static AST_Node * create_ast_ident_node(int offset) {
    AST_Node *node = (AST_Node *)calloc(1, sizeof(AST_Node));
    node->kind = AST_LVALUE;
    node->stack_offset = offset;
    node->left = NULL;
    node->right = NULL;
    return node;
}

static AST_Node * create_ast_num_node(int num_val) {
    AST_Node *node = (AST_Node *)calloc(1, sizeof(AST_Node));
    node->kind = AST_NUM;
    node->num_val = num_val;
    node->left = NULL;
    node->right = NULL;
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

static Token * try_eat_ident_token() {
    Token *tk = NULL;
    if (global_token_ptr->kind == TK_IDENT) {
        tk = global_token_ptr;
        global_token_ptr = global_token_ptr->next;
    }
    return tk;
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
     unit       := statement*
     statement  := expression ";"
     expression := assign
     assign     := equality ("=" assign)?
     equality   := comparison ("==" comparison | "!=" comparison)*
     comparison := add ("<" add | ">" add | "<=" add | ">=" add)*
     add        := mul ("+" mul | "-" mul)*
     mul        := unary ("*" unary | "/" unary | "%" unary)*
     unary      := ("+" | "-")? atom
     atom       := num | identifier | "(" expression ")"
*/ 

static AST_Node * parse_statement();
static AST_Node * parse_expression();
static AST_Node * parse_assign();
static AST_Node * parse_equality();
static AST_Node * parse_comparison();
static AST_Node * parse_add();
static AST_Node * parse_mul();
static AST_Node * parse_unary();
static AST_Node * parse_atom();

AST_Node ** parse_unit(char *code, Token *tokens) {
    AST_Node **ast_forest = (AST_Node **)malloc(sizeof(AST_Node *)*100);
    global_code = code;
    global_token_ptr = tokens;

    int i = 0;
    while (global_token_ptr->kind != TK_EOF) {
        ast_forest[i++] = parse_statement();
    }
    ast_forest[i] = NULL;
    return ast_forest;
}

// statement := expression ";"
static AST_Node * parse_statement() {
    AST_Node *node = parse_expression();
    if (!try_eat_op_token(";")) {
        panic_at(
            global_token_ptr->str, global_code,
            "Parsing error: Wrong Token (Got %s, expected %s[';'])",
            global_tk_repr[global_token_ptr->kind], global_tk_repr[TK_RESERVED]
        );
    }
    return node;
    
}

// expression := assign
static AST_Node * parse_expression() {
    return parse_assign();
}

// assign := equality ("=" assign)?
static AST_Node * parse_assign() {
    AST_Node *node = parse_equality();
    if (try_eat_op_token("=")) {
        node = create_ast_op_node(AST_ASSIGN, node, parse_assign());
    }
    return node;
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
            node = create_ast_op_node(AST_GR, parse_add(), node);
        } else if (try_eat_op_token(">=")) {
            node = create_ast_op_node(AST_GE, parse_add(), node);
        } else if (try_eat_op_token("<")) {
            node = create_ast_op_node(AST_GR, node, parse_add());
        } else if (try_eat_op_token("<=")) {
            node = create_ast_op_node(AST_GE, node, parse_add());
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

// atom := num | identifier | "(" expression ")"
static AST_Node * parse_atom() {
    Token * tk = try_eat_ident_token();
    if (tk) {
        return create_ast_ident_node((tk->str[0] - 'a' + 1) * 8);
    }
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
