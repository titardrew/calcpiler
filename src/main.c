#include "stdio.h"

#include "gen.h"
#include "lexer.h"
#include "parser.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Not enough arguments\n");
        return 1;
    }
    char *code = argv[1];

    Token *tokens = tokenize(code);
    AST_Node *ast_head = parse_code(code, tokens);
    gen_code(ast_head);

    return 0;
}
