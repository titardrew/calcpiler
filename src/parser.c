#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "parser.h"
#include "utils.h"

Token    *global_token_ptr;
char     *global_code;
Variable *global_locals;

static Variable * find_local_variable(Token *tk) {
    Variable *cur = global_locals;
    while (cur) {
        if (cur->len == tk->len && !memcmp(cur->name, tk->str, tk->len)) {
            return cur;
        }
        cur = cur->next;
    }
    return NULL;
}

static AST_Node * create_ast_op_node(AST_NodeKind kind,
        AST_Node *left, AST_Node *right) {
    AST_Node *node = (AST_Node *)calloc(1, sizeof(AST_Node));
    node->kind = kind;
    node->left = left;
    node->right = right;
    return node;
}

static AST_Node * create_ast_ident_node(Token *tk) {
    AST_Node *node = (AST_Node *)calloc(1, sizeof(AST_Node));
    node->kind = AST_LVALUE;
    node->left = NULL;
    node->right = NULL;

    Variable *var = find_local_variable(tk);
    if (var) {
        node->stack_offset = var->stack_offset;
    } else {
        node->stack_offset = global_locals->stack_offset + 8;

        var = (Variable *)calloc(1, sizeof(Variable));
        var->name = tk->str;
        var->len = tk->len;
        var->stack_offset = node->stack_offset;
        var->next = global_locals;

        global_locals = var;
    }
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

static void expect(char *sym) {
    if (!try_eat_op_token(sym)) {
        panic_at(
            global_token_ptr->str, global_code,
            "Parsing error: Wrong Token (Got %s, expected %s[\"%s\"])",
            global_tk_repr[global_token_ptr->kind],
            global_tk_repr[TK_RESERVED],
            sym
        );
    }
}

static Token * try_eat_ident_token() {
    Token *tk = NULL;
    if (global_token_ptr->kind == TK_IDENT) {
        tk = global_token_ptr;
        global_token_ptr = global_token_ptr->next;
    }
    return tk;
}

static Status try_eat_reserved_word_token(TokenKind kind) {
    if (global_token_ptr->kind != kind) {
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

#define CALC_MAX_TREE_WIDTH 100
#define CALC_MAX_TREE_DEPTH 100
static int _print_ast(AST_Node *tree, int is_left, int offset, int depth,
                      char s[CALC_MAX_TREE_DEPTH][CALC_MAX_TREE_WIDTH],
                      int* dst_depth) {
    char b[20];

    if (!tree) {
        *dst_depth = (*dst_depth > depth) ? (*dst_depth) : (depth);
        return 0;
    }

    sprintf(b, "%s", global_node_kind[tree->kind]);
    size_t width = strlen(b);

    int left  = _print_ast(
        tree->left,
        1,
        offset,
        depth + 1,
        s,
        dst_depth
    );

    int right = _print_ast(
        tree->right,
        0,
        offset + left + width,
        depth + 1,
        s,
        dst_depth
    );

    for (int i = 0; i < width; i++)
        s[depth][offset + left + i] = b[i];

    if (depth && is_left) {

        for (int i = 0; i < width + right; i++)
            s[depth - 1][offset + left + width/2 + i] = '-';

        s[depth - 1][offset + left + width/2] = '.';

    } else if (depth && !is_left) {

        for (int i = 0; i < left + width; i++)
            s[depth - 1][offset - width/2 + i] = '-';

        s[depth - 1][offset + left + width/2] = '.';
    }

    return left + width + right;
}

static void print_ast(AST_Node *tree) {
    char s[CALC_MAX_TREE_DEPTH][CALC_MAX_TREE_WIDTH];
    for (int i = 0; i < CALC_MAX_TREE_DEPTH; i++) {
        for (int j = 0; j < CALC_MAX_TREE_WIDTH - 1; j++)
            s[i][j] = ' ';
        s[i][CALC_MAX_TREE_WIDTH - 1] = '\0';
    }
    int max_depth = 0;
    _print_ast(tree, 0, 0, 0, s, &max_depth);
    for (int i = 0; i < max_depth; i++)
        fprintf(stderr, "%s\n", s[i]);
}

/* Grammar (EBNF):
     unit       := statement*
     statement  := expression ";"
                | "return" expression ";"
                | "if" "(" expression ")" statement ("else" statement)?
                | "while" "(" expression ")" statement
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

// unit := statement*
AST_Node ** parse_unit(char *code, Token *tokens) {
    AST_Node **ast_forest = (AST_Node **)malloc(sizeof(AST_Node *)*100);
    Variable var_head;
    var_head.stack_offset = 0;

    global_code = code;
    global_token_ptr = tokens;
    global_locals = &var_head;

    int i = 0;
    while (global_token_ptr->kind != TK_EOF) {
        ast_forest[i++] = parse_statement();
#ifdef DEBUG_PARSER
        print_ast(ast_forest[i - 1]);
#endif
    }
    ast_forest[i] = NULL;
    return ast_forest;
}

/*
     statement  := expression ";"
                | "return" expression ";"
                | "if" "(" expression ")" statement ("else" statement)?
                | "while" "(" expression ")" statement
*/
static AST_Node * parse_statement() {
    AST_Node *node;
    if (try_eat_reserved_word_token(TK_RET)) {
        node = (AST_Node *)calloc(1, sizeof(AST_Node));
        node->kind = AST_RET;
        node->right = NULL;
        node->left = parse_expression();
        expect(";");
    } else if (try_eat_reserved_word_token(TK_IF)) {
        expect("(");
        AST_Node *cond = parse_expression();
        expect(")");
        AST_Node *body = parse_statement();
        AST_Node *else_body = NULL;
        if (try_eat_reserved_word_token(TK_ELSE)) {
            else_body = parse_statement();
        }
        node = create_ast_op_node(AST_IF, body, else_body);
        node->cond = cond;
    } else if (try_eat_reserved_word_token(TK_WHILE)) {
        expect("(");
        AST_Node *cond = parse_expression();
        expect(")");
        node = create_ast_op_node(AST_WHILE, parse_statement(), NULL);
        node->cond = cond;
    } else if (try_eat_reserved_word_token(TK_FOR)) {
        AST_Node *init  = NULL;
        AST_Node *cond  = NULL;
        AST_Node *right = NULL;
        expect("(");
        if (!try_eat_op_token(";")) {
            init = parse_expression();
            expect(";");
        }
        if (!try_eat_op_token(";")) {
            cond = parse_expression();
            expect(";");
        }
        if (!try_eat_op_token(")")) {
            right = parse_expression();
            expect(")");
        }
        node = create_ast_op_node(AST_FOR, parse_statement(), right);
        node->init = init;
        node->cond = cond;
    } else {
        node = parse_expression();
        expect(";");
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
        return create_ast_ident_node(tk);
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
