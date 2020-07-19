#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "parser.h"
#include "utils.h"

char     *global_code;
Token    *global_token_ptr;
Scope    *global_cur_scope;
Declared *global_locals;
Function *global_functions;
Function *global_current_function;

static void create_scope() {
    Scope *scope = (Scope *)calloc(1, sizeof(Scope));
    scope->locals = (Declared *)calloc(1, sizeof(Declared));
    if (global_locals) {
        scope->locals->stack_offset = global_locals->stack_offset; 
    } else {
        scope->locals->stack_offset = 0; 
    }
    // NOTE(andrii): ^^^ might be useless (after calloc)

    scope->higher_scope = global_cur_scope;
    global_cur_scope = scope;
    global_locals = scope->locals;
}

static void exit_scope() {
    Scope *scope = global_cur_scope;
    global_cur_scope = scope->higher_scope;
    global_locals = global_cur_scope->locals;
    //free(scope);
}

void ast_nodevec_alloc(AST_NodeVec *vec, size_t capacity) {
    vec->size = 0;
    vec->capacity = capacity ? capacity : AST_NODEVEC_CAPACITY;
    vec->data = (AST_Node *)malloc(sizeof(AST_Node) * vec->capacity);
}

void ast_nodevec_reserve(AST_NodeVec *vec, size_t capacity) {
    if (capacity > vec->capacity) {
        vec->capacity = capacity;
        vec->data = (AST_Node *)realloc(vec, sizeof(AST_Node) * vec->capacity);
    } else if (capacity < vec->capacity) {
        panic("In ast_nodevec_reserve: new capacity"
              " has to be greater than old (%u<%u)", capacity, vec->capacity);
    }
}

void ast_nodevec_append(AST_NodeVec *vec, AST_Node *node) {
    if (!vec->data) {
        ast_nodevec_alloc(vec, 0);
    }
    ++vec->size;
    if (vec->size == vec->capacity) {
        ast_nodevec_reserve(vec, vec->capacity * 2);
    }
    vec->data[vec->size - 1] = *node;
}

static Declared * find_local_declared(Token *tk) {
    Declared *cur = global_locals;
    while (cur) {
        if (cur->len == tk->len && !memcmp(cur->name, tk->str, tk->len)) {
            return cur;
        }
        cur = cur->next;
    }
    return NULL;
}

static Declared * find_visible_declared(Token *tk) {
    Scope    *scope;
    Declared *cur;
    for (scope = global_cur_scope; scope; scope = scope->higher_scope) {
        for (cur = scope->locals; cur; cur = cur->next) {
            if (cur->len == tk->len && !memcmp(cur->name, tk->str, tk->len)) {
                return cur;
            }
        }
    }
    return NULL;
}

static Function * find_declared_function(Token *tk) {
    Function *cur = global_functions;
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
    node->has_undef_children = false;
    if (left) {
        node->has_undef_children |= left->has_undef_children;
    }
    if (right) {
        node->has_undef_children |= right->has_undef_children;
    }
    return node;
}

static AST_Node * create_ast_ident_node(Token *tk) {
    AST_Node *node = (AST_Node *)calloc(1, sizeof(AST_Node));
    node->kind = AST_LVALUE;
    node->left = NULL;
    node->right = NULL;

    // NOTE(andrii): variable shadowing is off, because variable
    //               declaration feature is not implemented yet!
    Declared *decl = find_visible_declared(tk);

    if (decl) {
        node->stack_offset = decl->stack_offset;
        node->has_undef_children = !decl->is_defined;
        node->decl = decl;
    } else {
        node->stack_offset = global_locals->stack_offset + 8;

        decl = (Declared *)calloc(1, sizeof(Declared));
        decl->name = tk->str;
        decl->len = tk->len;
        decl->stack_offset = node->stack_offset;
        decl->next = global_locals;

        global_locals = decl;
        global_cur_scope->locals = global_locals;
        node->has_undef_children = true;
        node->decl = decl;
    }
    return node;
}

static AST_Node * create_ast_fcall_node(Token *tk, AST_NodeVec *args) {
    AST_Node *node = (AST_Node *)calloc(1, sizeof(AST_Node));
    node->kind = AST_FCALL;
    node->left = NULL;
    node->right = NULL;
    node->args = args;
    node->has_undef_children = false;

    Function *func;

    bool is_recursive = false;

    if (tk->len == global_current_function->len &&
        !memcmp(tk->str, global_current_function->name, tk->len)) {
        // recursive call
        func = global_current_function;
        is_recursive = true;
    } else {
        func = find_declared_function(tk);
    }

    if (func && (func->node || is_recursive)) {
        node->func = func;
        if (args->size != func->n_args) {
            panic_at(tk->str, global_code,
                     "Parsing error: Arguments must match");
        }
    } else {
        node->func = (Function *)calloc(1, sizeof(Function));
        node->func->name = tk->str;
        node->func->len = tk->len;
        //fprintf(stderr, "warning: implicit func declaration: %.*s\n",
        //        tk->len, tk->str);
        /*
        panic_at(tk->str, global_code,
                 "Parsing error: Calling undeclared function");
        */
    }
    return node;
}

static AST_Node * create_ast_num_node(int num_val) {
    AST_Node *node = (AST_Node *)calloc(1, sizeof(AST_Node));
    node->kind = AST_NUM;
    node->num_val = num_val;
    node->left = NULL;
    node->right = NULL;
    node->has_undef_children = false;
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

    int left = _print_ast(
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

 unit       := function*
 function   := identifier "(" args ")" "{" statement* "}"
 args       := (expression ("," expression)* )?
 arg_names  := (identifier ("," identifier)* )?
 statement  := expression ";"
             | block
             | "return" expression ";"
             | "if" "(" expression ")" statement ("else" statement)?
             | "while" "(" expression ")" statement
             | "for" "(" expression? ";" expression? ";" expression? ")" statement
 block      := "{" statement* "}"
 expression := assign
 assign     := equality ("=" assign)?
 equality   := comparison ("==" comparison | "!=" comparison)*
 comparison := add ("<" add | ">" add | "<=" add | ">=" add)*
 add        := mul ("+" mul | "-" mul)*
 mul        := unary ("*" unary | "/" unary | "%" unary)*
 unary      := ("+" | "-")? atom
 atom       := num | identifier ("(" arg_names ")") | "(" expression ")"

*/ 

static AST_Node * parse_function();
static AST_Node * parse_statement();
static AST_Node * parse_block(bool);
static AST_Node * parse_expression();
static AST_Node * parse_assign();
static AST_Node * parse_equality();
static AST_Node * parse_comparison();
static AST_Node * parse_add();
static AST_Node * parse_mul();
static AST_Node * parse_unary();
static AST_Node * parse_atom();

static AST_NodeVec * parse_args();
static AST_NodeVec * parse_arg_names();

// unit := function*
AST_Node ** parse_unit(char *code, Token *tokens) {
    AST_Node **ast_forest = (AST_Node **)malloc(sizeof(AST_Node *)*100);

    global_code = code;
    global_token_ptr = tokens;

    create_scope();

    int i = 0;
    while (global_token_ptr->kind != TK_EOF) {
        ast_forest[i] = parse_function();
        if (ast_forest[i]) {
            ++i;
        }
#ifdef DEBUG_PARSER
        print_ast(ast_forest[i - 1]);
#endif
    }
    ast_forest[i] = NULL;
    return ast_forest;
}

// args := (expression ("," expression)* )?
static AST_NodeVec * parse_args() {
    AST_NodeVec *args = (AST_NodeVec *)calloc(1, sizeof(AST_NodeVec));
    ast_nodevec_alloc(args, 6);
    bool has_next_arg = !try_eat_op_token(")");
    while(has_next_arg) {
        AST_Node *arg = parse_expression();
        ast_nodevec_append(args, arg);
        has_next_arg = try_eat_op_token(",");
    }
    if (args->size > 0) expect(")");
    return args;
}


// arg_names := (ident ("," ident)* )?
static AST_NodeVec * parse_arg_names() {
    AST_NodeVec *args = (AST_NodeVec *)calloc(1, sizeof(AST_NodeVec));
    ast_nodevec_alloc(args, 6);
    expect("(");
    bool has_next_arg = !try_eat_op_token(")");
    while (has_next_arg) {
        AST_Node *arg;
        Token *tk = try_eat_ident_token();
        if (tk) {
            arg = create_ast_ident_node(tk);
            arg->decl->is_defined = true;
            arg->has_undef_children = false;
            // TODO(andrii): check if identifier is an eligible name
        } else {
            panic_at(tk->str, global_code,
                     "Parsing error: Invalid argument name");
        }
        ast_nodevec_append(args, arg);
        has_next_arg = try_eat_op_token(",");
    }
    if (args->size > 0) expect(")");
    return args;
}

// function := identifier "(" args ")" ( block | ";" )
static AST_Node * parse_function() {
    Function *func = NULL;

    Token *tk = try_eat_ident_token();
    if (!tk) {
        panic_at(
            tk->str, global_code,
            "Parsing error: Wrong token [Expected TK_IDENT "
            "in function delaration]"
        );
    }
    for (Function *fn = global_functions; fn; fn = fn->next) {
        if (tk->len == fn->len &&
                !memcmp(tk->str, fn->name, tk->len)) {
            func = fn;
            break;
        }
    }
    if (!func) {
        func = (Function *)calloc(1, sizeof(Function));
        func->name = tk->str;
        func->len = tk->len;
        func->next = global_functions;
        global_functions = func;
    }
    create_scope(); // FIXME(andrii): each declaration creates dangle scope
    AST_NodeVec *args = parse_arg_names();
    if (!try_eat_op_token(";")) {
        if (func->node) {
            panic_at(
                tk->str, global_code,
                "Parsing error: Redefinition of function!"
            );
        }
        global_current_function = func;
        func->n_args = args->size;
        func->node = parse_block(false);
        func->node->kind = AST_FUNC;
        func->node->func = func;
        func->node->args = args;
    } else {
        // TODO(andrii): dealloc args
    }
    exit_scope();
    return func->node;
}

/*
statement  := expression ";"
            | block
            | "return" expression ";"
            | "if" "(" expression ")" statement ("else" statement)?
            | "while" "(" expression ")" statement
            | "for" "(" expression? ";" expression? ";" expression? ")" statement
*/
static AST_Node * parse_statement() {
    AST_Node *node;

    node = parse_block(true);
    if (node) {
        return node;
    } else if (try_eat_reserved_word_token(TK_RET)) {
        node = create_ast_op_node(AST_RET, parse_expression(), NULL);
        expect(";");
    } else if (try_eat_reserved_word_token(TK_IF)) {
        create_scope();
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
        exit_scope();
    } else if (try_eat_reserved_word_token(TK_WHILE)) {
        create_scope();
        expect("(");
        AST_Node *cond = parse_expression();
        expect(")");
        node = create_ast_op_node(AST_WHILE, parse_statement(), NULL);
        node->cond = cond;
        exit_scope();
    } else if (try_eat_reserved_word_token(TK_FOR)) {
        create_scope();
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
        exit_scope();
    } else {
        node = parse_expression();
        expect(";");
    }

    return node;
}

// block := "{" statement* "}"
static AST_Node * parse_block(bool new_scope) {
    AST_Node *node = NULL;
    if (try_eat_op_token("{")) {
        if (new_scope) create_scope();
        node = (AST_Node *)calloc(1, sizeof(AST_Node));
        node->kind = AST_BLOCK;
        node->block = (AST_NodeVec *)calloc(1, sizeof(AST_NodeVec));
        while (!try_eat_op_token("}")) {
            AST_Node *node_ = parse_statement();
            ast_nodevec_append(node->block, node_);
            if (node_ == NULL) {
                panic_at(global_token_ptr->str, global_code, "WTF?!?");
            }
            //free(node_);
        }
        if (new_scope) exit_scope();
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
        if (node->left->decl) {
            node->left->decl->is_defined = true;
        } else {
            panic_at(global_token_ptr->str, global_code,
                     "Left-hand side must be LVALUE, not %s",
                     global_node_kind[node->left->kind]);
        }
        if (node->right->has_undef_children) {
            panic_at(global_token_ptr->str, global_code,
                     "Undefined variable usage spotted [%s]",
                     global_node_kind[node->right->kind]);
            // TODO: perform diagnostics, better error!
        }
    } else if (node->has_undef_children) {
        panic_at(global_token_ptr->str, global_code,
                 "Undefined variable usage spotted [%s]",
                 global_node_kind[node->kind]);
        // TODO: perform diagnostics, better error!
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

// atom := num
//       | identifier ("(" args ")")?
//       | "(" expression ")"
static AST_Node * parse_atom() {

    Token * tk = try_eat_ident_token();
    if (tk) {
        if (try_eat_op_token("(")) {
            AST_NodeVec *args = parse_args();
            return create_ast_fcall_node(tk, args);
        }
        return create_ast_ident_node(tk);
    } else if (try_eat_op_token("(")) {
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
