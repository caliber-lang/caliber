#define _POSIX_C_SOURCE 200809L
#include "lexer.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

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
    while (isspace((unsigned char)current_char(lex))) {
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

    while (isalnum((unsigned char)current_char(lex)) || current_char(lex) == '_') {
        buf[i++] = current_char(lex);
        advance(lex);
    }
    buf[i] = '\0';

    static const char *keywords[] = {
        "def", "data", "var", "if", "then", "else", "match", "with",
        "import", "type", "do", "for", "while", "let", "print", "alloc", "return", NULL
    };

    for (int k = 0; keywords[k]; k++) {
        if (strcmp(buf, keywords[k]) == 0) {
            return make_token(TOK_KEYWORD, buf);
        }
    }

    return make_token(TOK_ID, buf);
}

static token_t *read_number(lexer_t *lex) {
    char buf[256];
    int i = 0;

    while (isdigit((unsigned char)current_char(lex))) {
        buf[i++] = current_char(lex);
        advance(lex);
    }
    buf[i] = '\0';

    return make_token(TOK_INT, buf);
}

static token_t *read_string(lexer_t *lex) {
    advance(lex);
    char buf[1024];
    int i = 0;

    while (current_char(lex) != '"' && current_char(lex) != '\0') {
        char c = current_char(lex);
        if (c == '\\') {
            advance(lex);
            char esc = current_char(lex);
            switch (esc) {
                case 'n': buf[i++] = '\n'; break;
                case 't': buf[i++] = '\t'; break;
                case '\\': buf[i++] = '\\'; break;
                case '"': buf[i++] = '"'; break;
                default: buf[i++] = esc; break;
            }
            advance(lex);
        } else {
            buf[i++] = c;
            advance(lex);
        }
    }
    buf[i] = '\0';

    advance(lex);
    return make_token(TOK_STRING, buf);
}

token_t *lexer_next(lexer_t *lex) {
    for (;;) {
        skip_whitespace(lex);
        if (current_char(lex) == ';') {
            skip_comment(lex);
            continue;
        }
        break;
    }

    char c = current_char(lex);

    if (c == '\0') return make_token(TOK_EOF, NULL);
    if (isalpha((unsigned char)c) || c == '_') return read_id_or_keyword(lex);
    if (isdigit((unsigned char)c)) return read_number(lex);
    if (c == '"') return read_string(lex);

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
        return make_token(TOK_EQUALS, "=");
    }
    if (c == '!') {
        advance(lex);
        if (current_char(lex) == '=') {
            advance(lex);
            return make_token(TOK_NEQ, "!=");
        }
        fprintf(stderr, "unexpected character: !\n");
        exit(1);
    }

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

const char *token_type_name(token_type_t type) {
    switch (type) {
        case TOK_EOF: return "EOF";
        case TOK_KEYWORD: return "KEYWORD";
        case TOK_ID: return "ID";
        case TOK_INT: return "INT";
        case TOK_STRING: return "STRING";
        case TOK_EQUALS: return "EQUALS";
        case TOK_AT: return "AT";
        case TOK_COLON: return "COLON";
        case TOK_COLONEQUAL: return "COLONEQUAL";
        case TOK_ARROW: return "ARROW";
        case TOK_COLONMINUS: return "COLONMINUS";
        case TOK_DOT: return "DOT";
        case TOK_LPAREN: return "LPAREN";
        case TOK_RPAREN: return "RPAREN";
        case TOK_LBRACE: return "LBRACE";
        case TOK_RBRACE: return "RBRACE";
        case TOK_LBRACKET: return "LBRACKET";
        case TOK_RBRACKET: return "RBRACKET";
        case TOK_COMMA: return "COMMA";
        case TOK_PIPE: return "PIPE";
        case TOK_PLUS: return "PLUS";
        case TOK_MINUS: return "MINUS";
        case TOK_STAR: return "STAR";
        case TOK_SLASH: return "SLASH";
        case TOK_EQ: return "EQ";
        case TOK_NEQ: return "NEQ";
        case TOK_LT: return "LT";
        case TOK_GT: return "GT";
        case TOK_LTE: return "LTE";
        case TOK_GTE: return "GTE";
        default: return "UNKNOWN";
    }
}
