#include "stdio.h"
#include "stdlib.h"
#include "ctype.h"
#include "string.h"
#include "stdbool.h"

#include "lexer.h"
#include "utils.h"

static inline bool is_ident_char(char c) {
    return isalpha(c) || c == '_';
}

static int read_ident(char *start, char **dst) {
    char *p;
    for (p = start; is_ident_char(*p) || isdigit(*p); ++p);
    *dst = p;
    return p - start;
}

Token * create_token(TokenKind kind, Token *current_token,
                     char *str, int len) {
    Token *new_token = (Token *)calloc(1, sizeof(Token));
    new_token->kind = kind;
    new_token->str = str;
    new_token->len = len;
    current_token->next = new_token;
    return new_token;
}

Status lex_reserved_word(TokenKind kind, int chars_left,
                         char **p, Token **cur) {
    const char *word = global_tk_repr[kind];
    int len = strlen(word);
    if (chars_left > len && !memcmp(*p, word, len)
         && !is_ident_char((*p)[len]) && !isdigit((*p)[len])) {
        *cur = create_token(kind, *cur, *p, len);
#ifdef DEBUG_LEXER
        fprintf(stderr, "TK[%s]->", word);
#endif
        *p += len;
        return SUCCESS;
    }
    return FAILURE;
}

Token * tokenize(char *code) {
    char *p = code;
    Token head;
    head.next = NULL;
    head.len = 0;  // C99 does not allow struct init :(
    Token *current_token = &head;
    size_t chars_left = strlen(p);

    while (*p) {

        if (isspace(*p)) {
            ++p;
            --chars_left;
            continue;
        }
        chars_left -= current_token->len;

        // Reserved words
        if (lex_reserved_word(TK_RET, chars_left, &p, &current_token))
            continue;
        if (lex_reserved_word(TK_IF, chars_left, &p, &current_token))
            continue;
        if (lex_reserved_word(TK_ELSE, chars_left, &p, &current_token))
            continue;
        if (lex_reserved_word(TK_WHILE, chars_left, &p, &current_token))
            continue;
        if (lex_reserved_word(TK_FOR, chars_left, &p, &current_token))
            continue;

        // Identifier
        if (chars_left >= 1 && is_ident_char(*p)) {
            current_token = create_token(TK_IDENT, current_token, p, 0);
            char *num_begin = p;
            current_token->len = read_ident(num_begin, &p);
#ifdef DEBUG_LEXER
            fprintf(stderr, "TK_IDENT[%.*s]->", current_token->len, num_begin);
#endif
            continue;
        }

        // 2-char ops
        if (chars_left >= 2 && (
                has_prefix(p, "<=") || has_prefix(p, ">=") ||
                has_prefix(p, "==") || has_prefix(p, "!="))) {
            current_token = create_token(TK_RESERVED, current_token, p, 2);
#ifdef DEBUG_LEXER
            fprintf(stderr, "TK_RESERVED[%.*s]->", 2, p);
#endif
            p += 2;
            continue;
        }

        // 1-char ops
        if (chars_left >= 1 && strchr("()+-*/%<>=;", *p)) {
            current_token = create_token(TK_RESERVED, current_token, p, 1);
#ifdef DEBUG_LEXER
            fprintf(stderr, "TK_RESERVED[%c]->", p[0]);
#endif
            ++p;
            continue;
        }

        if (isdigit(*p)) {
            current_token = create_token(TK_NUM, current_token, p, 0);
            char *num_begin = p;
            current_token->num_val = strtol(num_begin, &p, 10);
            current_token->len = p - num_begin;
#ifdef DEBUG_LEXER
            fprintf(stderr, "TK_NUM[%d]->", current_token->num_val);
#endif
            continue;
        }

#ifdef DEBUG_LEXER
            fprintf(stderr, "\n");
#endif
        panic_at(p, code, "Lexing error: Can't figure out token.");
    }

    create_token(TK_EOF, current_token, p-1, 0);
#ifdef DEBUG_LEXER
            fprintf(stderr, "TK_EOF\n");
#endif
    return head.next;
}
