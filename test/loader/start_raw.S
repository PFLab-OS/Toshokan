.code64
.section ".text.boot"
.extern main
.global entry
entry:
	jmp main
  ret
