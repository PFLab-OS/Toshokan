/* ported from newlib */
/*
 * ====================================================
 * Copyright (C) 2007 by Ellips BV. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */
  
  .global toshokan_setjmp
  .global toshokan_longjmp
  .global toshokan_setjmp_with_wait
  
toshokan_setjmp: 
  movq    %rbx,  0 (%rdi)
  movq    %rbp,  8 (%rdi)
  movq    %r12, 16 (%rdi)
  movq    %r13, 24 (%rdi)
  movq    %r14, 32 (%rdi)
  movq    %r15, 40 (%rdi)
  leaq    8 (%rsp), %rax
  movq    %rax, 48 (%rdi)
  movq    (%rsp), %rax
  movq    %rax, 56 (%rdi)
  movq    $0, %rax
  ret

toshokan_setjmp_with_wait: 
  movq    %rbx,  0 (%rdi)
  movq    %rbp,  8 (%rdi)
  movq    %r12, 16 (%rdi)
  movq    %r13, 24 (%rdi)
  movq    %r14, 32 (%rdi)
  movq    %r15, 40 (%rdi)
  leaq    8 (%rsp), %rax
  movq    %rax, 48 (%rdi)
  movq    (%rsp), %rax
  movq    %rax, 56 (%rdi)
  movq    $1,   64(%rdi)	
1:
  cmpq    $0,   72(%rdi)
  je      1b
  movq    $0,   72(%rdi)
  movq    $0, %rax
  ret

toshokan_longjmp:  
  movq    %rsi, %rax        /* Return value */
  movq     8 (%rdi), %rbp
  movq    48 (%rdi), %rsp
  pushq   56 (%rdi)
  movq     0 (%rdi), %rbx
  movq    16 (%rdi), %r12
  movq    24 (%rdi), %r13
  movq    32 (%rdi), %r14
  movq    40 (%rdi), %r15
  ret

 /* end of ported code */
