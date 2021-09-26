# file: directives.s

.global b
.extern f

.section text
.word c
.equ a, 0
.skip 4
b: .word f
.equ c, 2
.skip 1
.word e
d: .equ e, 0x01

.section data
.word 0x123F
.word 6
.skip 2

.global a

.end