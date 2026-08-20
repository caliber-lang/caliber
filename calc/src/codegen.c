#include "codegen.h"
#include "typetab.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *name;
    int offset;
} sym_t;

typedef struct {
    char *name;
    char *type_name;
    int is_ref;              /* 1 if reference, 0 if stake */
    char *target_stake;      /* if ref, name of stake it references */
} stake_info_t;

static sym_t syms[256];
static int sym_count;

static stake_info_t stakes[256];
static int stake_count;

static char *string_pool[256];
static int string_count;

static char current_epilogue[128];
static int label_counter;

static const char *arg_regs[6] = {
    "%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"
};

static void declare_symbol(const char *name) {
    for (int i = 0; i < sym_count; i++) {
        if (strcmp(syms[i].name, name) == 0) return;
    }
    syms[sym_count].name = (char *)name;
    syms[sym_count].offset = (sym_count + 1) * 8;
    sym_count++;
}

static int find_symbol(const char *name) {
    for (int i = 0; i < sym_count; i++) {
        if (strcmp(syms[i].name, name) == 0) return syms[i].offset;
    }
    fprintf(stderr, "codegen error: unknown symbol %s\n", name);
    exit(1);
    return -1;
}

static void declare_stake(const char *name, const char *type_name, int is_ref, const char *target) {
    for (int i = 0; i < stake_count; i++) {
        if (strcmp(stakes[i].name, name) == 0) return;
    }
    stakes[stake_count].name = (char *)name;
    stakes[stake_count].type_name = (char *)type_name;
    stakes[stake_count].is_ref = is_ref;
    stakes[stake_count].target_stake = target ? (char *)target : NULL;
    stake_count++;
}

static stake_info_t *find_stake_info(const char *name) {
    for (int i = 0; i < stake_count; i++) {
        if (strcmp(stakes[i].name, name) == 0) return &stakes[i];
    }
    return NULL;
}

static const char *get_stake_type(const char *name) {
    stake_info_t *info = find_stake_info(name);
    if (info && info->type_name) return info->type_name;
    fprintf(stderr, "codegen error: no known type for stake %s\n", name);
    exit(1);
    return NULL;
}

static const char *try_get_stake_type(const char *name) {
    stake_info_t *info = find_stake_info(name);
    if (info) return info->type_name;
    return NULL;
}

static const char *resolve_static_type(node_t *e) {
    if (e->type == NODE_STAKEREF) return get_stake_type(e->name);
    if (e->type == NODE_IDENT) return get_stake_type(e->name);
    if (e->type == NODE_FIELDACCESS) {
        fprintf(stderr, "codegen error: nested field access not yet supported\n");
        exit(1);
    }
    fprintf(stderr, "codegen error: cannot resolve static type of expression\n");
    exit(1);
    return NULL;
}

static void collect_symbols_stmt(node_t *stmt) {
    if (stmt->type == NODE_VARDECL || stmt->type == NODE_STAKEDECL || stmt->type == NODE_REFDECL) {
        declare_symbol(stmt->name);
    }
    if (stmt->type == NODE_IF) {
        for (int i = 0; i < stmt->right->child_count; i++) {
            collect_symbols_stmt(stmt->right->children[i]);
        }
        if (stmt->third) {
            for (int i = 0; i < stmt->third->child_count; i++) {
                collect_symbols_stmt(stmt->third->children[i]);
            }
        }
    }
}

static void collect_symbols(node_t *fn) {
    for (int i = 0; i < fn->param_count; i++) {
        declare_symbol(fn->param_names[i]);
    }
    for (int i = 0; i < fn->child_count; i++) {
        collect_symbols_stmt(fn->children[i]);
    }
}

static void collect_strings(node_t *n) {
    if (!n) return;

    if (n->type == NODE_STRING) {
        string_pool[string_count] = n->sval;
        n->ival = string_count;
        string_count++;
        return;
    }

    for (int i = 0; i < n->child_count; i++) {
        collect_strings(n->children[i]);
    }
    for (int i = 0; i < n->field_count; i++) {
        collect_strings(n->field_values[i]);
    }
    collect_strings(n->left);
    collect_strings(n->right);
    collect_strings(n->third);
}

static void codegen_stmt(FILE *f, node_t *s);

static void codegen_expr(FILE *f, node_t *e) {
    switch (e->type) {
        case NODE_INT:
            fprintf(f, "    movq $%ld, %%rax\n", e->ival);
            break;
        case NODE_STRING:
            fprintf(f, "    leaq .LSTR%ld(%%rip), %%rax\n", e->ival);
            break;
        case NODE_IDENT: {
            int off = find_symbol(e->name);
            fprintf(f, "    movq -%d(%%rbp), %%rax\n", off);
            break;
        }
        case NODE_STAKEREF: {
            int off = find_symbol(e->name);
            fprintf(f, "    movq -%d(%%rbp), %%rax\n", off);
            break;
        }
        case NODE_REFREF: {
            /* ref @stake: load the reference value (pointer + generation) */
            int off = find_symbol(e->stake_name);
            fprintf(f, "    movq -%d(%%rbp), %%rax\n", off);
            break;
        }
        case NODE_FIELDACCESS: {
            const char *base_type = resolve_static_type(e->left);
            int field_off = typetab_field_offset(base_type, e->name);
            codegen_expr(f, e->left);
            fprintf(f, "    movq %d(%%rax), %%rax\n", field_off);
            break;
        }
        case NODE_BINOP: {
            codegen_expr(f, e->left);
            fprintf(f, "    pushq %%rax\n");
            codegen_expr(f, e->right);
            fprintf(f, "    movq %%rax, %%rbx\n");
            fprintf(f, "    popq %%rax\n");
            switch (e->op) {
                case '+': fprintf(f, "    addq %%rbx, %%rax\n"); break;
                case '-': fprintf(f, "    subq %%rbx, %%rax\n"); break;
                case '*': fprintf(f, "    imulq %%rbx, %%rax\n"); break;
                case '/':
                    fprintf(f, "    cqto\n");
                    fprintf(f, "    idivq %%rbx\n");
                    break;
            }
            break;
        }
        case NODE_CMP: {
            codegen_expr(f, e->left);
            fprintf(f, "    pushq %%rax\n");
            codegen_expr(f, e->right);
            fprintf(f, "    movq %%rax, %%rbx\n");
            fprintf(f, "    popq %%rax\n");
            fprintf(f, "    cmpq %%rbx, %%rax\n");
            const char *setcc;
            if (strcmp(e->cmp_op, "==") == 0) setcc = "sete";
            else if (strcmp(e->cmp_op, "!=") == 0) setcc = "setne";
            else if (strcmp(e->cmp_op, "<") == 0) setcc = "setl";
            else if (strcmp(e->cmp_op, ">") == 0) setcc = "setg";
            else if (strcmp(e->cmp_op, "<=") == 0) setcc = "setle";
            else if (strcmp(e->cmp_op, ">=") == 0) setcc = "setge";
            else {
                fprintf(stderr, "codegen error: unknown comparator %s\n", e->cmp_op);
                exit(1);
            }
            fprintf(f, "    %s %%al\n", setcc);
            fprintf(f, "    movzbq %%al, %%rax\n");
            break;
        }
        case NODE_CALL: {
            for (int i = 0; i < e->child_count; i++) {
                codegen_expr(f, e->children[i]);
                fprintf(f, "    pushq %%rax\n");
            }
            for (int i = e->child_count - 1; i >= 0; i--) {
                fprintf(f, "    popq %s\n", arg_regs[i]);
            }
            fprintf(f, "    call %s\n", e->name);
            break;
        }
        default:
            fprintf(stderr, "codegen error: bad expr node (type=%d)\n", e->type);
            exit(1);
    }
}

static void codegen_stmt(FILE *f, node_t *s) {
    switch (s->type) {
        case NODE_VARDECL: {
            codegen_expr(f, s->left);
            int off = find_symbol(s->name);
            fprintf(f, "    movq %%rax, -%d(%%rbp)\n", off);
            if (s->left->type == NODE_STAKEREF || s->left->type == NODE_IDENT) {
                const char *t = try_get_stake_type(s->left->name);
                if (t) set_stake_type(s->name, t);
            }
            break;
        }
        case NODE_STAKEDECL: {
            node_t *lit = s->left;
            type_info_t *info = typetab_lookup(lit->type_name);
            if (!info) {
                fprintf(stderr, "codegen error: unknown type %s\n", lit->type_name);
                exit(1);
            }

            /* allocate memory for object */
            fprintf(f, "    movl $%d, %%edi\n", info->total_size);
            fprintf(f, "    call malloc\n");
            fprintf(f, "    movq %%rax, %%rbx\n");
            
            /* store generation counter (initially 0) */
            fprintf(f, "    movq $0, (%%rbx)\n");

            /* initialize fields */
            for (int i = 0; i < lit->field_count; i++) {
                fprintf(f, "    pushq %%rbx\n");
                codegen_expr(f, lit->field_values[i]);
                fprintf(f, "    movq %%rax, %%rcx\n");
                fprintf(f, "    popq %%rbx\n");
                int field_off = typetab_field_offset(lit->type_name, lit->field_names[i]);
                fprintf(f, "    movq %%rcx, %d(%%rbx)\n", field_off);
            }

            /* store stake (pointer) in local */
            int off = find_symbol(s->name);
            fprintf(f, "    movq %%rbx, -%d(%%rbp)\n", off);
            
            declare_stake(s->name, lit->type_name, 0, NULL);
            break;
        }
        case NODE_REFDECL: {
            /* var ref_name <- ref @stake */
            stake_info_t *target_info = find_stake_info(s->stake_name);
            if (!target_info) {
                fprintf(stderr, "codegen error: unknown stake %s\n", s->stake_name);
                exit(1);
            }

            /* load stake pointer */
            int stake_off = find_symbol(s->stake_name);
            fprintf(f, "    movq -%d(%%rbp), %%rax\n", stake_off);

            /* store reference (for now, just the pointer) */
            int ref_off = find_symbol(s->name);
            fprintf(f, "    movq %%rax, -%d(%%rbp)\n", ref_off);

            declare_stake(s->name, target_info->type_name, 1, s->stake_name);
            break;
        }
        case NODE_MUTATION: {
            if (s->left->type == NODE_FIELDACCESS) {
                node_t *fa = s->left;
                const char *base_type = resolve_static_type(fa->left);
                int field_off = typetab_field_offset(base_type, fa->name);
                codegen_expr(f, fa->left);
                fprintf(f, "    pushq %%rax\n");
                codegen_expr(f, s->right);
                fprintf(f, "    movq %%rax, %%rcx\n");
                fprintf(f, "    popq %%rax\n");
                fprintf(f, "    movq %%rcx, %d(%%rax)\n", field_off);
            } else {
                codegen_expr(f, s->right);
                int off = find_symbol(s->left->name);
                fprintf(f, "    movq %%rax, -%d(%%rbp)\n", off);
            }
            break;
        }
        case NODE_PRINT: {
            if (s->left->type == NODE_STRING) {
                fprintf(f, "    leaq .LSTR%ld(%%rip), %%rdi\n", s->left->ival);
                fprintf(f, "    movl $0, %%eax\n");
                fprintf(f, "    call printf\n");
            } else {
                codegen_expr(f, s->left);
                fprintf(f, "    movq %%rax, %%rsi\n");
                fprintf(f, "    leaq .LFMT_INT(%%rip), %%rdi\n");
                fprintf(f, "    movl $0, %%eax\n");
                fprintf(f, "    call printf\n");
            }
            break;
        }
        case NODE_RETURN: {
            codegen_expr(f, s->left);
            fprintf(f, "    jmp %s\n", current_epilogue);
            break;
        }
        case NODE_IF: {
            int id = label_counter++;
            codegen_expr(f, s->left);
            fprintf(f, "    testq %%rax, %%rax\n");
            fprintf(f, "    jz .Lelse%d\n", id);
            for (int i = 0; i < s->right->child_count; i++) {
                codegen_stmt(f, s->right->children[i]);
            }
            fprintf(f, "    jmp .Lendif%d\n", id);
            fprintf(f, ".Lelse%d:\n", id);
            if (s->third) {
                for (int i = 0; i < s->third->child_count; i++) {
                    codegen_stmt(f, s->third->children[i]);
                }
            }
            fprintf(f, ".Lendif%d:\n", id);
            break;
        }
        default:
            fprintf(stderr, "codegen error: bad stmt node (type=%d)\n", s->type);
            exit(1);
    }
}

static void codegen_funcdef(FILE *f, node_t *fn) {
    sym_count = 0;
    stake_count = 0;
    label_counter = 0;
    collect_symbols(fn);

    int stack_size = sym_count * 8;
    if (stack_size % 16 != 0) stack_size += 16 - (stack_size % 16);
    if (stack_size == 0) stack_size = 16;

    snprintf(current_epilogue, sizeof(current_epilogue), ".Lret_%s", fn->name);

    fprintf(f, "%s:\n", fn->name);
    fprintf(f, "    pushq %%rbp\n");
    fprintf(f, "    movq %%rsp, %%rbp\n");
    fprintf(f, "    subq $%d, %%rsp\n", stack_size);

    for (int i = 0; i < fn->param_count; i++) {
        int off = find_symbol(fn->param_names[i]);
        fprintf(f, "    movq %s, -%d(%%rbp)\n", arg_regs[i], off);
        if (fn->param_types[i]) {
            set_stake_type(fn->param_names[i], fn->param_types[i]);
        }
    }

    for (int i = 0; i < fn->child_count; i++) {
        codegen_stmt(f, fn->children[i]);
    }

    fprintf(f, "    movl $0, %%eax\n");
    fprintf(f, "%s:\n", current_epilogue);
    fprintf(f, "    movq %%rbp, %%rsp\n");
    fprintf(f, "    popq %%rbp\n");
    fprintf(f, "    ret\n");
}

void codegen_generate(node_t *program, const char *out_path) {
    typetab_reset();
    for (int i = 0; i < program->child_count; i++) {
        if (program->children[i]->type == NODE_DATADEF) {
            typetab_register(program->children[i]);
        }
    }

    string_count = 0;
    collect_strings(program);

    FILE *f = fopen(out_path, "w");
    if (!f) {
        fprintf(stderr, "codegen error: cannot open %s\n", out_path);
        exit(1);
    }

    fprintf(f, "    .section .rodata\n");
    fprintf(f, ".LFMT_INT:\n    .string \"%%ld\\n\"\n");
    for (int i = 0; i < string_count; i++) {
        fprintf(f, ".LSTR%d:\n    .string \"%s\"\n", i, string_pool[i]);
    }

    fprintf(f, "    .section .text\n");
    fprintf(f, "    .globl main\n");

    for (int i = 0; i < program->child_count; i++) {
        if (program->children[i]->type == NODE_FUNCDEF) {
            codegen_funcdef(f, program->children[i]);
        }
    }

    fclose(f);
}
