# file: ulaz1.s

.global a
.global b
.global c
.extern d

.section data
    .word 0xff20
c:  .word a

.section text
    ldr r1, $0x05
a:  ldr r0, %b
b:  .word c

.end