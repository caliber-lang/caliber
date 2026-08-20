#include "x86_64.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *name;
    int offset;
} local_t;

static local_t locals[256];
static int local_count;
static int string_count;

static void fail(const char *msg) {
    fprintf(stderr, "x86-64 backend: %s\n", msg);
    exit(1);
}

static void local_add(const char *name) {
    for (int i = 0; i < local_count; i++) {
        if (strcmp(locals[i].name, name) == 0) return;
    }

    size_t len = strlen(name) + 1;
    locals[local_count].name = malloc(len);
    memcpy(locals[local_count].name, name, len);
    locals[local_count].offset = (local_count + 1) * 8;
    local_count++;
}

static int local_find(const char *name) {
    for (int i = 0; i < local_count; i++) {
        if (strcmp(locals[i].name, name) == 0)
            return locals[i].offset;
    }

    fprintf(stderr, "x86-64 backend: unknown local %s\n", name);
    exit(1);
}

static void collect_locals(caliber_program_t *p) {
    for (int i = 0; i < p->count; i++) {
        caliber_inst_t *in = &p->items[i];

        if (in->type == CI_LOAD || in->type == CI_STORE)
            local_add(in->a);
    }
}

static void free_locals(void) {
    for (int i = 0; i < local_count; i++)
        free(locals[i].name);

    local_count = 0;
}

static void emit_binop(FILE *f, const char *op) {
    fprintf(f, "    popq %%rbx\n");

    if (strcmp(op, "+") == 0)
        fprintf(f, "    addq %%rbx, %%rax\n");
    else if (strcmp(op, "-") == 0)
        fprintf(f, "    subq %%rbx, %%rax\n");
    else if (strcmp(op, "*") == 0)
        fprintf(f, "    imulq %%rbx, %%rax\n");
    else if (strcmp(op, "/") == 0) {
        fprintf(f, "    xchgq %%rax, %%rbx\n");
        fprintf(f, "    cqto\n");
        fprintf(f, "    idivq %%rbx\n");
    } else {
        fail("unknown binary operator");
    }
}

static void emit_cmp(FILE *f, const char *op) {
    fprintf(f, "    popq %%rbx\n");
    fprintf(f, "    cmpq %%rax, %%rbx\n");

    if (strcmp(op, "==") == 0)
        fprintf(f, "    sete %%al\n");
    else if (strcmp(op, "!=") == 0)
        fprintf(f, "    setne %%al\n");
    else if (strcmp(op, "<") == 0)
        fprintf(f, "    setl %%al\n");
    else if (strcmp(op, ">") == 0)
        fprintf(f, "    setg %%al\n");
    else if (strcmp(op, "<=") == 0)
        fprintf(f, "    setle %%al\n");
    else if (strcmp(op, ">=") == 0)
        fprintf(f, "    setge %%al\n");
    else
        fail("unknown comparison");

    fprintf(f, "    movzbq %%al, %%rax\n");
}

static void emit_function(FILE *f, caliber_program_t *p, int *ip) {
    caliber_inst_t *fn = &p->items[*ip];

    local_count = 0;
    collect_locals(p);

    int stack = local_count * 8;

    if (stack % 16 != 0)
        stack += 16 - stack % 16;

    if (stack == 0)
        stack = 16;

    fprintf(f, "    .globl %s\n", fn->a);
    fprintf(f, "    .type %s, @function\n", fn->a);
    fprintf(f, "%s:\n", fn->a);
    fprintf(f, "    pushq %%rbp\n");
    fprintf(f, "    movq %%rsp, %%rbp\n");
    fprintf(f, "    subq $%d, %%rsp\n", stack);

    int args[6] = {
        0, 1, 2, 3, 4, 5
    };

    const char *regs[6] = {
        "%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"
    };

    for (int i = 0; i < fn->n && i < 6; i++) {
        char name[32];
        snprintf(name, sizeof(name), "%d", i);
        local_add(name);
        fprintf(f, "    movq %s, -%d(%%rbp)\n",
                regs[args[i]], local_find(name));
    }

    (*ip)++;

    for (; *ip < p->count; (*ip)++) {
        caliber_inst_t *in = &p->items[*ip];

        if (in->type == CI_END)
            break;

        switch (in->type) {
            case CI_INT:
                fprintf(f, "    movq $%ld, %%rax\n", in->value);
                fprintf(f, "    pushq %%rax\n");
                break;

            case CI_LOAD:
                fprintf(f, "    movq -%d(%%rbp), %%rax\n",
                        local_find(in->a));
                fprintf(f, "    pushq %%rax\n");
                break;

            case CI_STORE:
                fprintf(f, "    popq %%rax\n");
                fprintf(f, "    movq %%rax, -%d(%%rbp)\n",
                        local_find(in->a));
                break;

            case CI_BINOP:
                fprintf(f, "    popq %%rax\n");
                emit_binop(f, in->a);
                fprintf(f, "    pushq %%rax\n");
                break;

            case CI_CMP:
                fprintf(f, "    popq %%rax\n");
                emit_cmp(f, in->a);
                fprintf(f, "    pushq %%rax\n");
                break;

            case CI_PRINT:
                fprintf(f, "    popq %%rsi\n");
                fprintf(f, "    leaq .LFMT_INT(%%rip), %%rdi\n");
                fprintf(f, "    xorl %%eax, %%eax\n");
                fprintf(f, "    call printf@PLT\n");
                break;

            case CI_RETURN:
                fprintf(f, "    popq %%rax\n");
                fprintf(f, "    leave\n");
                fprintf(f, "    ret\n");
                break;

            case CI_JUMP:
                fprintf(f, "    jmp %s\n", in->a);
                break;

            case CI_JUMP_ZERO:
                fprintf(f, "    popq %%rax\n");
                fprintf(f, "    testq %%rax, %%rax\n");
                fprintf(f, "    jz %s\n", in->a);
                break;

            case CI_LABEL:
                fprintf(f, "%s:\n", in->a);
                break;

            default:
                fail("instruction not implemented");
        }
    }

    fprintf(f, "    movl $0, %%eax\n");
    fprintf(f, "    leave\n");
    fprintf(f, "    ret\n");

    free_locals();
}

void x86_64_emit(caliber_program_t *p, FILE *out) {
    fprintf(out, "    .section .rodata\n");
    fprintf(out, ".LFMT_INT:\n");
    fprintf(out, "    .string \"%%ld\\n\"\n");

    fprintf(out, "    .section .text\n");

    for (int i = 0; i < p->count; i++) {
        if (p->items[i].type == CI_FUNC)
            emit_function(out, p, &i);
    }

    fprintf(out, "    .section .note.GNU-stack,\"\",@progbits\n");

    (void)string_count;
}
