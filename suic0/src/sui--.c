#include "sui--.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void expr(FILE *f, node_t *n) {
    if (!n) return;

    switch (n->type) {
        case NODE_INT:
            fprintf(f, "i 0 0 %ld\n", n->ival);
            break;

        case NODE_STRING:
            fprintf(f, "s %ld\n", n->ival);
            break;

        case NODE_IDENT:
        case NODE_ANCHORREF:
            fprintf(f, "r %s\n", n->name);
            break;

        case NODE_FIELDACCESS:
            expr(f, n->left);
            fprintf(f, "g %s\n", n->name);
            break;

        case NODE_BINOP:
            expr(f, n->left);
            expr(f, n->right);
            fprintf(f, "b %c\n", n->op);
            break;

        case NODE_CMP:
            expr(f, n->left);
            expr(f, n->right);
            fprintf(f, "c %s\n", n->cmp_op);
            break;

        default:
            fprintf(stderr, "sui-- error: invalid expression %d\n", n->type);
            exit(1);
    }
}

static void stmt(FILE *f, node_t *n) {
    if (!n) return;

    switch (n->type) {
        case NODE_VARDECL:
            expr(f, n->left);
            fprintf(f, "v %s\n", n->name);
            break;

        case NODE_ANCHORDECL:
            fprintf(f, "a %s %s\n", n->name, n->left->type_name);
            for (int i = 0; i < n->left->field_count; i++) {
                expr(f, n->left->field_values[i]);
                fprintf(
                    f,
                    "i %s %d\n",
                    n->left->field_names[i],
                    i
                );
            }
            break;

        case NODE_MUTATION:
            expr(f, n->right);
            if (n->left->type == NODE_FIELDACCESS) {
                fprintf(f, "m %s\n", n->left->name);
            } else {
                fprintf(f, "m %s\n", n->left->name);
            }
            break;

        case NODE_PRINT:
            expr(f, n->left);
            fprintf(f, "p\n");
            break;

        case NODE_RETURN:
            expr(f, n->left);
            fprintf(f, "q\n");
            break;

        case NODE_IF:
            expr(f, n->left);
            fprintf(f, "j\n");

            for (int i = 0; i < n->right->child_count; i++) {
                stmt(f, n->right->children[i]);
            }

            if (n->third) {
                fprintf(f, "j\n");
                for (int i = 0; i < n->third->child_count; i++) {
                    stmt(f, n->third->children[i]);
                }
            }
            break;

        default:
            fprintf(stderr, "sui-- error: invalid statement %d\n", n->type);
            exit(1);
    }
}

static void func(FILE *f, node_t *n) {
    fprintf(
        f,
        "f %s %d %d\n",
        n->name,
        n->param_count,
        n->child_count
    );

    for (int i = 0; i < n->child_count; i++) {
        stmt(f, n->children[i]);
    }

    fprintf(f, "e\n");
}

void sui_emit(node_t *program, FILE *out) {
    for (int i = 0; i < program->child_count; i++) {
        node_t *n = program->children[i];

        if (n->type == NODE_FUNCDEF) {
            func(out, n);
        }
    }
}
