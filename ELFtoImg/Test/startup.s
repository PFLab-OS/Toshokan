.global _start

_start:
    ldr x30, =stack_top
    adr x29, _start
    add x30, x30, x29
    mov sp, x30
    bl c_entry
    b .
