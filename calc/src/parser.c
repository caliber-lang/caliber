#define _POSIX_C_SOURCE 200809L
#include "parser.h"
#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    lexer_t *lex;
    token_t *cur;
} parser_t;

static void perror_exit(const char *msg) {
    fprintf(stderr, "parse error: %s\n", msg);
    exit(1);
}

static void adv(parser_t *p) {
    token_free(p->cur);
    p->cur = lexer_next(p->lex);
}

static void expect_type(parser_t *p, token_type_t t) {
    if (p->cur->type != t) perror_exit("unexpected token type");
}

static int is_kw(parser_t *p, const char *kw) {
    return p->cur->type == TOK_KEYWORD && strcmp(p->cur->value, kw) == 0;
}

static node_t *parse_expr(parser_t *p);
static node_t *parse_stmt(parser_t *p);

static node_t *parse_struct_lit(parser_t *p, const char *type_name) {
    node_t *node = node_new(NODE_STRUCTLIT);
    node->type_name = strdup(type_name);
    node->field_count = 0;

    expect_type(p, TOK_LBRACE);
    adv(p);

    if (p->cur->type != TOK_RBRACE) {
        for (;;) {
            if (node->field_count >= MAX_FIELDS) perror_exit("too many fields in struct literal");
            expect_type(p, TOK_ID);
            char *fname = strdup(p->cur->value);
            adv(p);
            expect_type(p, TOK_COLON);
            adv(p);
            node_t *fval = parse_expr(p);
            node->field_names[node->field_count] = fname;
            node->field_values[node->field_count] = fval;
            node->field_count++;
            if (p->cur->type == TOK_COMMA) { adv(p); continue; }
            break;
        }
    }

    expect_type(p, TOK_RBRACE);
    adv(p);

    return node;
}

static node_t *parse_call_args(parser_t *p, node_t *call) {
    adv(p);
    if (p->cur->type != TOK_RPAREN) {
        for (;;) {
            node_t *arg = parse_expr(p);
            node_add_child(call, arg);
            if (call->child_count > 6) perror_exit("too many call arguments");
            if (p->cur->type == TOK_COMMA) { adv(p); continue; }
            break;
        }
    }
    expect_type(p, TOK_RPAREN);
    adv(p);
    return call;
}

static node_t *parse_postfix(parser_t *p, node_t *node) {
    while (p->cur->type == TOK_DOT) {
        adv(p);
        expect_type(p, TOK_ID);
        node_t *access = node_new(NODE_FIELDACCESS);
        access->left = node;
        access->name = strdup(p->cur->value);
        adv(p);
        node = access;
    }
    return node;
}

static node_t *parse_factor(parser_t *p) {
    node_t *node;

    if (p->cur->type == TOK_INT) {
        node = node_new(NODE_INT);
        node->ival = atol(p->cur->value);
        adv(p);
        return node;
    }
    if (p->cur->type == TOK_STRING) {
        node = node_new(NODE_STRING);
        node->sval = strdup(p->cur->value);
        adv(p);
        return node;
    }

    /* ref @stake reference to a stake */
    if (is_kw(p, "ref")) {
        adv(p);
        expect_type(p, TOK_AT);
        adv(p);
        expect_type(p, TOK_ID);
        node = node_new(NODE_REFREF);
        node->stake_name = strdup(p->cur->value);
        adv(p);
        return node;
    }

    if (p->cur->type == TOK_AT) {
        adv(p);
        expect_type(p, TOK_ID);
        node = node_new(NODE_STAKEREF);
        node->name = strdup(p->cur->value);
        adv(p);
        return parse_postfix(p, node);
    }
    if (p->cur->type == TOK_ID) {
        char *name = strdup(p->cur->value);
        adv(p);
        if (p->cur->type == TOK_LPAREN) {
            node = node_new(NODE_CALL);
            node->name = name;
            node = parse_call_args(p, node);
            return parse_postfix(p, node);
        }
        node = node_new(NODE_IDENT);
        node->name = name;
        return parse_postfix(p, node);
    }
    if (p->cur->type == TOK_LPAREN) {
        adv(p);
        node = parse_expr(p);
        expect_type(p, TOK_RPAREN);
        adv(p);
        return parse_postfix(p, node);
    }

    perror_exit("expected factor");
    return NULL;
}

static node_t *parse_term(parser_t *p) {
    node_t *node = parse_factor(p);

    while (p->cur->type == TOK_STAR || p->cur->type == TOK_SLASH) {
        char op = p->cur->type == TOK_STAR ? '*' : '/';
        adv(p);
        node_t *right = parse_factor(p);
        node_t *n = node_new(NODE_BINOP);
        n->op = op;
        n->left = node;
        n->right = right;
        node = n;
    }

    return node;
}

static node_t *parse_addsub(parser_t *p) {
    node_t *node = parse_term(p);

    while (p->cur->type == TOK_PLUS || p->cur->type == TOK_MINUS) {
        char op = p->cur->type == TOK_PLUS ? '+' : '-';
        adv(p);
        node_t *right = parse_term(p);
        node_t *n = node_new(NODE_BINOP);
        n->op = op;
        n->left = node;
        n->right = right;
        node = n;
    }

    return node;
}

static const char *cmp_op_str(token_type_t t) {
    switch (t) {
        case TOK_EQ: return "==";
        case TOK_NEQ: return "!=";
        case TOK_LT: return "<";
        case TOK_GT: return ">";
        case TOK_LTE: return "<=";
        case TOK_GTE: return ">=";
        default: return NULL;
    }
}

static int is_cmp_tok(token_type_t t) {
    return t == TOK_EQ || t == TOK_NEQ || t == TOK_LT ||
           t == TOK_GT || t == TOK_LTE || t == TOK_GTE;
}

static node_t *parse_expr(parser_t *p) {
    node_t *node = parse_addsub(p);

    if (is_cmp_tok(p->cur->type)) {
        const char *op = cmp_op_str(p->cur->type);
        adv(p);
        node_t *right = parse_addsub(p);
        node_t *n = node_new(NODE_CMP);
        n->cmp_op = strdup(op);
        n->left = node;
        n->right = right;
        node = n;
    }

    return node;
}

static int starts_stmt(parser_t *p) {
    if (is_kw(p, "var")) return 1;
    if (is_kw(p, "print")) return 1;
    if (is_kw(p, "return")) return 1;
    if (is_kw(p, "if")) return 1;
    if (p->cur->type == TOK_AT) return 1;
    if (p->cur->type == TOK_ID) return 1;
    return 0;
}

static node_t *parse_lvalue_tail(parser_t *p, node_t *base) {
    node_t *target = base;
    char *last_field = NULL;

    while (p->cur->type == TOK_DOT) {
        adv(p);
        expect_type(p, TOK_ID);
        if (last_field) {
            node_t *access = node_new(NODE_FIELDACCESS);
            access->left = target;
            access->name = last_field;
            target = access;
        }
        last_field = strdup(p->cur->value);
        adv(p);
    }

    node_t *field_target = node_new(NODE_FIELDACCESS);
    field_target->left = target;
    field_target->name = last_field;

    node_t *mut = node_new(NODE_MUTATION);
    mut->left = field_target;

    expect_type(p, TOK_COLONEQUAL);
    adv(p);
    mut->right = parse_expr(p);

    return mut;
}

static node_t *parse_block(parser_t *p) {
    node_t *block = node_new(NODE_BLOCK);
    while (starts_stmt(p) && !is_kw(p, "def") && !is_kw(p, "data") &&
           !is_kw(p, "else")) {
        node_t *stmt = parse_stmt(p);
        node_add_child(block, stmt);
    }
    return block;
}

static node_t *parse_stmt(parser_t *p) {
    node_t *node;

    if (is_kw(p, "if")) {
        adv(p);
        node_t *cond = parse_expr(p);
        if (!is_kw(p, "then")) perror_exit("expected then");
        adv(p);
        node_t *then_block = parse_block(p);
        node_t *else_block = NULL;
        if (is_kw(p, "else")) {
            adv(p);
            else_block = parse_block(p);
        }
        node = node_new(NODE_IF);
        node->left = cond;
        node->right = then_block;
        node->third = else_block;
        return node;
    }

    if (is_kw(p, "var")) {
        adv(p);
        expect_type(p, TOK_ID);
        char *name = strdup(p->cur->value);
        adv(p);
        expect_type(p, TOK_COLONMINUS);
        adv(p);

        /* check for ref keyword */
        if (is_kw(p, "ref")) {
            adv(p);
            expect_type(p, TOK_AT);
            adv(p);
            expect_type(p, TOK_ID);
            char *stake_name = strdup(p->cur->value);
            adv(p);

            node = node_new(NODE_REFDECL);
            node->name = name;
            node->stake_name = stake_name;
            node->is_ref = 1;
            return node;
        }

        node_t *expr = parse_expr(p);
        node = node_new(NODE_VARDECL);
        node->name = name;
        node->left = expr;
        return node;
    }

    if (is_kw(p, "return")) {
        adv(p);
        node_t *expr = parse_expr(p);
        node = node_new(NODE_RETURN);
        node->left = expr;
        return node;
    }

    if (p->cur->type == TOK_AT) {
        adv(p);
        expect_type(p, TOK_ID);
        char *name = strdup(p->cur->value);
        adv(p);

        /* @name <- alloc Type { ... } */
        if (p->cur->type == TOK_COLONMINUS) {
            adv(p);
            if (!is_kw(p, "alloc")) perror_exit("expected alloc");
            adv(p);
            expect_type(p, TOK_ID);
            char *type_name = strdup(p->cur->value);
            adv(p);
            node_t *lit = parse_struct_lit(p, type_name);
            free(type_name);
            node = node_new(NODE_STAKEDECL);
            node->name = name;
            node->left = lit;
            return node;
        }

        /* @name.field := value */
        if (p->cur->type == TOK_DOT) {
            node_t *base = node_new(NODE_STAKEREF);
            base->name = name;
            return parse_lvalue_tail(p, base);
        }

        /* @name := value */
        if (p->cur->type == TOK_COLONEQUAL) {
            adv(p);
            node_t *expr = parse_expr(p);
            node_t *base = node_new(NODE_STAKEREF);
            base->name = name;
            node = node_new(NODE_MUTATION);
            node->left = base;
            node->right = expr;
            return node;
        }

        perror_exit("expected <- or := or . after stake name");
    }

    if (p->cur->type == TOK_ID) {
        char *name = strdup(p->cur->value);
        adv(p);
        node_t *base = node_new(NODE_IDENT);
        base->name = name;

        if (p->cur->type == TOK_DOT) {
            return parse_lvalue_tail(p, base);
        }

        expect_type(p, TOK_COLONEQUAL);
        adv(p);
        node_t *expr = parse_expr(p);
        node = node_new(NODE_MUTATION);
        node->left = base;
        node->right = expr;
        return node;
    }

    if (is_kw(p, "print")) {
        adv(p);
        node_t *expr = parse_expr(p);
        node = node_new(NODE_PRINT);
        node->left = expr;
        return node;
    }

    perror_exit("expected statement");
    return NULL;
}

static node_t *parse_datadef(parser_t *p) {
    if (!is_kw(p, "data")) perror_exit("expected data");
    adv(p);
    expect_type(p, TOK_ID);
    char *name = strdup(p->cur->value);
    adv(p);
    expect_type(p, TOK_EQUALS);
    adv(p);
    expect_type(p, TOK_LBRACE);
    adv(p);

    node_t *node = node_new(NODE_DATADEF);
    node->name = name;
    node->field_count = 0;

    if (p->cur->type != TOK_RBRACE) {
        for (;;) {
            if (node->field_count >= MAX_FIELDS) perror_exit("too many fields in data def");
            expect_type(p, TOK_ID);
            char *fname = strdup(p->cur->value);
            adv(p);
            expect_type(p, TOK_COLON);
            adv(p);
            expect_type(p, TOK_ID);
            char *ftype = strdup(p->cur->value);
            adv(p);
            node->field_names[node->field_count] = fname;
            node->field_types[node->field_count] = ftype;
            node->field_count++;
            if (p->cur->type == TOK_COMMA) { adv(p); continue; }
            break;
        }
    }

    expect_type(p, TOK_RBRACE);
    adv(p);

    return node;
}

static node_t *parse_funcdef(parser_t *p) {
    if (!is_kw(p, "def")) perror_exit("expected def");
    adv(p);
    expect_type(p, TOK_ID);
    char *name = strdup(p->cur->value);
    adv(p);

    node_t *fn = node_new(NODE_FUNCDEF);
    fn->name = name;
    fn->param_count = 0;

    if (p->cur->type == TOK_LPAREN) {
        adv(p);
        if (p->cur->type != TOK_RPAREN) {
            for (;;) {
                if (fn->param_count >= MAX_PARAMS) perror_exit("too many params");
                if (p->cur->type == TOK_AT) {
                    adv(p);
                }
                expect_type(p, TOK_ID);
                char *pname = strdup(p->cur->value);
                adv(p);
                char *ptype = NULL;
                if (p->cur->type == TOK_COLON) {
                    adv(p);
                    expect_type(p, TOK_ID);
                    ptype = strdup(p->cur->value);
                    adv(p);
                }
                fn->param_names[fn->param_count] = pname;
                fn->param_types[fn->param_count] = ptype;
                fn->param_count++;
                if (p->cur->type == TOK_COMMA) { adv(p); continue; }
                break;
            }
        }
        expect_type(p, TOK_RPAREN);
        adv(p);
    }

    if (p->cur->type == TOK_ARROW) {
        adv(p);
        expect_type(p, TOK_ID);
        fn->ret_type = strdup(p->cur->value);
        adv(p);
    }

    expect_type(p, TOK_EQUALS);
    adv(p);

    while (starts_stmt(p) && !is_kw(p, "def") && !is_kw(p, "data")) {
        node_t *stmt = parse_stmt(p);
        node_add_child(fn, stmt);
    }

    return fn;
}

node_t *parser_parse(const char *source) {
    parser_t p;
    p.lex = lexer_new(source);
    p.cur = lexer_next(p.lex);

    node_t *prog = node_new(NODE_PROGRAM);

    while (p.cur->type != TOK_EOF) {
        if (is_kw(&p, "data")) {
            node_t *dd = parse_datadef(&p);
            node_add_child(prog, dd);
            continue;
        }
        if (is_kw(&p, "def")) {
            node_t *fn = parse_funcdef(&p);
            node_add_child(prog, fn);
            continue;
        }
        perror_exit("expected def or data at top level");
    }

    token_free(p.cur);
    lexer_free(p.lex);

    return prog;
}
