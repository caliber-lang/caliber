#ifndef LEXER_H
#define LEXER_H

#include <stdint.h>

typedef enum {
    TOK_EOF,
    TOK_KEYWORD,
    TOK_ID,
    TOK_INT,
    TOK_STRING,
    
    TOK_AT,           // @
    TOK_COLON,        // :
    TOK_COLONEQUAL,   // :=
    TOK_ARROW,        // ->
    TOK_COLONMINUS,   // <-
    TOK_DOT,          // .
    TOK_LPAREN,       // (
    TOK_RPAREN,       // )
    TOK_LBRACE,       // {
    TOK_RBRACE,       // }
    TOK_LBRACKET,     // [
    TOK_RBRACKET,     // ]
    TOK_COMMA,        // ,
    TOK_PIPE,         // |
    TOK_PLUS,         // +
    TOK_MINUS,        // -
    TOK_STAR,         // *
    TOK_SLASH,        // /
    TOK_EQ,           // ==
    TOK_NEQ,          // !=
    TOK_LT,           // 
    TOK_GT,           // >
    TOK_LTE,          // <=
    TOK_GTE,          // >=
} token_type_t;

typedef struct {
    token_type_t type;
    char *value;
    int line;
    int col;
} token_t;

typedef struct {
    char *source;
    int pos;
    int line;
    int col;
} lexer_t;

lexer_t *lexer_new(const char *source);
token_t *lexer_next(lexer_t *lex);
void lexer_free(lexer_t *lex);
void token_free(token_t *tok);

#endif
