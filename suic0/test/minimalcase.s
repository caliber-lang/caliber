    .section .rodata
.LFMT_INT:
    .string "%ld\n"
.LSTR0:
    .string "yes"
.LSTR1:
    .string "no"
    .section .text
    .globl main
main:
    pushq %rbp
    movq %rsp, %rbp
    subq $16, %rsp
    movq $1, %rax
    pushq %rax
    movq $2, %rax
    movq %rax, %rbx
    popq %rax
    cmpq %rbx, %rax
    setl %al
    movzbq %al, %rax
    testq %rax, %rax
    jz .Lelse0
    leaq .LSTR0(%rip), %rdi
    movl $0, %eax
    call printf
    jmp .Lendif0
.Lelse0:
    leaq .LSTR1(%rip), %rdi
    movl $0, %eax
    call printf
.Lendif0:
    movl $0, %eax
.Lret_main:
    movq %rbp, %rsp
    popq %rbp
    ret
