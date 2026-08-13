#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "usage: suic <file.sui>\n");
        return 1;
    }
    
    const char *filename = argv[1];
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("fopen");
        return 1;
    }
    
    // read file
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    char *source = malloc(size + 1);
    fread(source, 1, size, f);
    source[size] = '\0';
    fclose(f);
    
    // lex
    lexer_t *lex = lexer_new(source);
    token_t *tok;
    
    printf("tokens:\n");
    while ((tok = lexer_next(lex))->type != TOK_EOF) {
        printf("  %d: %s\n", tok->type, tok->value ? tok->value : "(null)");
        token_free(tok);
    }
    
    lexer_free(lex);
    free(source);
    
    return 0;
}
