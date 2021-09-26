# file: jumps.s

.global c
.equ b, 3

.section text
	jmp 3
	jmp a
	jmp %a
	jmp *[r1]
	jmp *[r2 + 4]
	jmp *[r3 + b]
	c:
	jmp *10
	jmp *a

.section data
	.skip 4
	a: .word 0xFC
	.word b

.end