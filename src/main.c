#include "stdio.h"
#include "string.h"

#include "gen_x86.h"
#include "lexer.h"
#include "parser.h"

int main(int argc, char *argv[]) {

    if (argc < 2) {
        fprintf(stderr, "Not enough arguments\n");
        return 1;
    }
    char *input = argv[1];
    char *code;
    if (strlen(input) > 2 && argv[1][0] == '-' && argv[1][1] == 'i') {
        input += 2;
        while (*input == ' ') input++;
        FILE *fp = fopen(input, "r");
        if (!fp) {
            fprintf(stderr, "Wrong filepath: %s\n", input);
            return 1;
        }
        fseek(fp , 0L, SEEK_END);
        long f_size = ftell(fp);
        rewind(fp);
        code = calloc( 1, f_size + 1 );
        if (!code) {
            fclose(fp);
            fputs("memory alloc fails", stderr);
            exit(1);
        }
        if (!fread(code, f_size, 1, fp)) {
            fclose(fp);
            free(code);
            fputs("entire read fails", stderr);
            exit(1);
        }
        fclose(fp);
    } else {
        code = input;
    }

    Token *tokens = tokenize(code);
    AST_Node **ast_forest = parse_unit(code, tokens);
    gen_code(ast_forest);

    return 0;
}
