#include "codegen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *name;
    int offset;
} sym_t;

static sym_t syms[256];
static int sym_count;

static char *string_pool[256];
static int string_count;

static char current_epilogue[128];

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
}

static void collect_symbols_stmt(node_t *stmt) {
    if (stmt->type == NODE_VARDECL || stmt->type == NODE_ANCHORDECL) {
        declare_symbol(stmt->name);
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
    collect_strings(n->left);
    collect_strings(n->right);
}

static void codegen_expr(FILE *f, node_t *e) {
    switch (e->type) {
        case NODE_INT:
            fprintf(f, "    movq $%ld, %%rax\n", e->ival);
            break;
        case NODE_IDENT: {
            int off = find_symbol(e->name);
            fprintf(f, "    movq -%d(%%rbp), %%rax\n", off);
            break;
        }
        case NODE_ANCHORREF: {
            int off = find_symbol(e->name);
            fprintf(f, "    movq -%d(%%rbp), %%rax\n", off);
            fprintf(f, "    movq (%%rax), %%rax\n");
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
            fprintf(stderr, "codegen error: bad expr node\n");
            exit(1);
    }
}

static void codegen_stmt(FILE *f, node_t *s) {
    switch (s->type) {
        case NODE_VARDECL: {
            codegen_expr(f, s->left);
            int off = find_symbol(s->name);
            fprintf(f, "    movq %%rax, -%d(%%rbp)\n", off);
            break;
        }
        case NODE_ANCHORDECL: {
            codegen_expr(f, s->left);
            fprintf(f, "    pushq %%rax\n");
            fprintf(f, "    movl $8, %%edi\n");
            fprintf(f, "    call malloc\n");
            fprintf(f, "    popq %%rbx\n");
            fprintf(f, "    movq %%rbx, (%%rax)\n");
            int off = find_symbol(s->name);
            fprintf(f, "    movq %%rax, -%d(%%rbp)\n", off);
            break;
        }
        case NODE_MUTATION: {
            if (s->op == '@') {
                codegen_expr(f, s->left);
                fprintf(f, "    movq %%rax, %%rbx\n");
                int off = find_symbol(s->name);
                fprintf(f, "    movq -%d(%%rbp), %%rax\n", off);
                fprintf(f, "    movq %%rbx, (%%rax)\n");
            } else {
                codegen_expr(f, s->left);
                int off = find_symbol(s->name);
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
        default:
            fprintf(stderr, "codegen error: bad stmt node\n");
            exit(1);
    }
}

static void codegen_funcdef(FILE *f, node_t *fn) {
    sym_count = 0;
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
        codegen_funcdef(f, program->children[i]);
    }

    fclose(f);
}
