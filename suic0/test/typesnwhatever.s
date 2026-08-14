    .section .rodata
.LFMT_INT:
    .string "%ld\n"
.LSTR0:
    .string "Alice"
    .section .text
    .globl main
main:
    pushq %rbp
    movq %rsp, %rbp
    subq $16, %rsp
    movl $24, %edi
    call malloc
    movq %rax, %rbx
    movq $0, (%rbx)
    pushq %rbx
    leaq .LSTR0(%rip), %rax
    movq %rax, %rcx
    popq %rbx
    movq %rcx, 8(%rbx)
    pushq %rbx
    movq $25, %rax
    movq %rax, %rcx
    popq %rbx
    movq %rcx, 16(%rbx)
    movq %rbx, -8(%rbp)
    movq -8(%rbp), %rax
    movq 16(%rax), %rax
    movq %rax, %rsi
    leaq .LFMT_INT(%rip), %rdi
    movl $0, %eax
    call printf
    movq -8(%rbp), %rax
    pushq %rax
    movq $26, %rax
    movq %rax, %rcx
    popq %rax
    movq %rcx, 16(%rax)
    movq -8(%rbp), %rax
    movq 16(%rax), %rax
    movq %rax, %rsi
    leaq .LFMT_INT(%rip), %rdi
    movl $0, %eax
    call printf
    movq -8(%rbp), %rax
    movq %rax, -16(%rbp)
    movq -16(%rbp), %rax
    movq 16(%rax), %rax
    movq %rax, %rsi
    leaq .LFMT_INT(%rip), %rdi
    movl $0, %eax
    call printf
    movl $0, %eax
.Lret_main:
    movq %rbp, %rsp
    popq %rbp
    ret
