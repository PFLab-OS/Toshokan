.code64
.balign 4
.text
.irp    param,0,1,2,3,4,5,6,7,9,15,16,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34
  .global i\param
  i\param:
  pushq $0
  pushq $\param
  jmp ix
  .endr

.irp    param,8,10,11,12,13,14,17
  .global i\param
  i\param:
  pushq $\param
  jmp ix
  .endr

ix:
  pushq %r15
  pushq %r14
  pushq %r13
  pushq %r12
  pushq %r11
  pushq %r10
  pushq %r9
  pushq %r8
  pushq %rdi
  pushq %rsi
  pushq %rbp
  pushq %rdx
  pushq %rcx
  pushq %rbx
  pushq %rax
  movq  %rsp, %rdi
  call handle_int
  popq %rax
  popq %rbx
  popq %rcx
  popq %rdx
  popq %rbp
  popq %rsi
  popq %rdi
  popq %r8
  popq %r9
  popq %r10
  popq %r11
  popq %r12
  popq %r13
  popq %r14
  popq %r15
  add $16, %rsp
  iretq

.data
.globl idt_vectors
idt_vectors:
.irp    param,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34
  .quad i\param
.endr
