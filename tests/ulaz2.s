# file: ulaz2.s

.extern a
.extern b
.global d
.equ e, 3

.section text
    .word a
d:  .word b
    ldr r0, d

.section data
    .word d
    .skip 3
    .word 5
    .word e

.end