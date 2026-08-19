#ifndef AST_H
#define AST_H

typedef enum {
    NODE_PROGRAM,
    NODE_FUNCDEF,
    NODE_VARDECL,
    NODE_ANCHORDECL,
    NODE_MUTATION,
    NODE_PRINT,
    NODE_RETURN,
    NODE_CALL,
    NODE_INT,
    NODE_STRING,
    NODE_IDENT,
    NODE_ANCHORREF,
    NODE_BINOP,
    NODE_DATADEF,
    NODE_STRUCTLIT,
    NODE_FIELDACCESS,
    NODE_CMP,
    NODE_IF,
    NODE_BLOCK,
} node_type_t;

typedef struct node node_t;

#define MAX_PARAMS 8
#define MAX_FIELDS 16

struct node {
    node_type_t type;
    char *name;
    long ival;
    char *sval;
    char op;
    node_t *left;
    node_t *right;
    node_t *third;
    node_t **children;
    int child_count;
    int child_cap;

    char *param_names[MAX_PARAMS];
    char *param_types[MAX_PARAMS];
    int param_count;
    char *ret_type;

    char *field_names[MAX_FIELDS];
    char *field_types[MAX_FIELDS];
    node_t *field_values[MAX_FIELDS];
    int field_count;

    char *type_name;

    char *cmp_op;
};

node_t *node_new(node_type_t type);
void node_add_child(node_t *parent, node_t *child);

#endif
