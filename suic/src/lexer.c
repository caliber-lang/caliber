#include "lexer.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

lexer_t *lexer_new(const char *source) {
    lexer_t *lex = malloc(sizeof(lexer_t));
    lex->source = strdup(source);
    lex->pos = 0;
    lex->line = 1;
    lex->col = 1;
    return lex;
}

static char current_char(lexer_t *lex) {
    return lex->source[lex->pos];
}

static char peek_char(lexer_t *lex) {
    return lex->source[lex->pos + 1];
}

static void advance(lexer_t *lex) {
    if (current_char(lex) == '\n') {
        lex->line++;
        lex->col = 1;
    } else {
        lex->col++;
    }
    lex->pos++;
}

static void skip_whitespace(lexer_t *lex) {
    while (isspace(current_char(lex))) {
        advance(lex);
    }
}

static void skip_comment(lexer_t *lex) {
    if (current_char(lex) == ';') {
        while (current_char(lex) != '\n' && current_char(lex) != '\0') {
            advance(lex);
        }
    }
}

static token_t *make_token(token_type_t type, const char *value) {
    token_t *tok = malloc(sizeof(token_t));
    tok->type = type;
    tok->value = value ? strdup(value) : NULL;
    return tok;
}

static token_t *read_id_or_keyword(lexer_t *lex) {
    char buf[256];
    int i = 0;
    
    while (isalnum(current_char(lex)) || current_char(lex) == '_') {
        buf[i++] = current_char(lex);
        advance(lex);
    }
    buf[i] = '\0';
    
    // check if keyword
    if (strcmp(buf, "def") == 0 || strcmp(buf, "data") == 0 ||
        strcmp(buf, "var") == 0 || strcmp(buf, "if") == 0 ||
        strcmp(buf, "then") == 0 || strcmp(buf, "else") == 0 ||
        strcmp(buf, "match") == 0 || strcmp(buf, "with") == 0) {
        return make_token(TOK_KEYWORD, buf);
    }
    
    return make_token(TOK_ID, buf);
}

static token_t *read_number(lexer_t *lex) {
    char buf[256];
    int i = 0;
    
    while (isdigit(current_char(lex))) {
        buf[i++] = current_char(lex);
        advance(lex);
    }
    buf[i] = '\0';
    
    return make_token(TOK_INT, buf);
}

static token_t *read_string(lexer_t *lex) {
    advance(lex);  // skip opening "
    char buf[1024];
    int i = 0;
    
    while (current_char(lex) != '"' && current_char(lex) != '\0') {
        buf[i++] = current_char(lex);
        advance(lex);
    }
    buf[i] = '\0';
    
    advance(lex);  // skip closing "
    return make_token(TOK_STRING, buf);
}

token_t *lexer_next(lexer_t *lex) {
    skip_whitespace(lex);
    skip_comment(lex);
    skip_whitespace(lex);
    
    char c = current_char(lex);
    
    if (c == '\0') return make_token(TOK_EOF, NULL);
    if (isalpha(c) || c == '_') return read_id_or_keyword(lex);
    if (isdigit(c)) return read_number(lex);
    if (c == '"') return read_string(lex);
    
    // single/double char operators
    if (c == '@') { advance(lex); return make_token(TOK_AT, "@"); }
    if (c == ':') {
        advance(lex);
        if (current_char(lex) == '=') {
            advance(lex);
            return make_token(TOK_COLONEQUAL, ":=");
        }
        return make_token(TOK_COLON, ":");
    }
    if (c == '<') {
        advance(lex);
        if (current_char(lex) == '-') {
            advance(lex);
            return make_token(TOK_COLONMINUS, "<-");
        }
        if (current_char(lex) == '=') {
            advance(lex);
            return make_token(TOK_LTE, "<=");
        }
        return make_token(TOK_LT, "<");
    }
    if (c == '-') {
        advance(lex);
        if (current_char(lex) == '>') {
            advance(lex);
            return make_token(TOK_ARROW, "->");
        }
        return make_token(TOK_MINUS, "-");
    }
    if (c == '=') {
        advance(lex);
        if (current_char(lex) == '=') {
            advance(lex);
            return make_token(TOK_EQ, "==");
        }
        return make_token(TOK_KEYWORD, "=");
    }
    if (c == '!') {
        advance(lex);
        if (current_char(lex) == '=') {
            advance(lex);
            return make_token(TOK_NEQ, "!=");
        }
    }
    
    // single char tokens
    if (c == '.') { advance(lex); return make_token(TOK_DOT, "."); }
    if (c == '(') { advance(lex); return make_token(TOK_LPAREN, "("); }
    if (c == ')') { advance(lex); return make_token(TOK_RPAREN, ")"); }
    if (c == '{') { advance(lex); return make_token(TOK_LBRACE, "{"); }
    if (c == '}') { advance(lex); return make_token(TOK_RBRACE, "}"); }
    if (c == '[') { advance(lex); return make_token(TOK_LBRACKET, "["); }
    if (c == ']') { advance(lex); return make_token(TOK_RBRACKET, "]"); }
    if (c == ',') { advance(lex); return make_token(TOK_COMMA, ","); }
    if (c == '|') { advance(lex); return make_token(TOK_PIPE, "|"); }
    if (c == '+') { advance(lex); return make_token(TOK_PLUS, "+"); }
    if (c == '*') { advance(lex); return make_token(TOK_STAR, "*"); }
    if (c == '/') { advance(lex); return make_token(TOK_SLASH, "/"); }
    if (c == '>') {
        advance(lex);
        if (current_char(lex) == '=') {
            advance(lex);
            return make_token(TOK_GTE, ">=");
        }
        return make_token(TOK_GT, ">");
    }
    
    fprintf(stderr, "unexpected character: %c\n", c);
    exit(1);
}

void lexer_free(lexer_t *lex) {
    free(lex->source);
    free(lex);
}

void token_free(token_t *tok) {
    if (tok->value) free(tok->value);
    free(tok);
}
