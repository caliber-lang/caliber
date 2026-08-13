    .section .rodata
.LFMT_INT:
    .string "%ld\n"
.LSTR0:
    .string "hello sui"
    .section .text
    .globl main
main:
    pushq %rbp
    movq %rsp, %rbp
    subq $32, %rsp
    movq $5, %rax
    movq %rax, -8(%rbp)
    movq $10, %rax
    movq %rax, -16(%rbp)
    movq -8(%rbp), %rax
    pushq %rax
    movq -16(%rbp), %rax
    movq %rax, %rbx
    popq %rax
    addq %rbx, %rax
    movq %rax, %rsi
    leaq .LFMT_INT(%rip), %rdi
    movl $0, %eax
    call printf
    leaq .LSTR0(%rip), %rdi
    movl $0, %eax
    call printf
    movq $42, %rax
    pushq %rax
    movl $8, %edi
    call malloc
    popq %rbx
    movq %rbx, (%rax)
    movq %rax, -24(%rbp)
    movq -24(%rbp), %rax
    movq (%rax), %rax
    movq %rax, %rsi
    leaq .LFMT_INT(%rip), %rdi
    movl $0, %eax
    call printf
    movq $99, %rax
    movq %rax, %rbx
    movq -24(%rbp), %rax
    movq %rbx, (%rax)
    movq -24(%rbp), %rax
    movq (%rax), %rax
    movq %rax, %rsi
    leaq .LFMT_INT(%rip), %rdi
    movl $0, %eax
    call printf
    movq %rbp, %rsp
    popq %rbp
    xorl %eax, %eax
    ret
