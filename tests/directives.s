# file: directives.s

.global b
.extern f

.section first
.word c
.equ a, 3
.skip 4
b: .word f

.section second
.equ c, 2
.skip 1
.word e
d: .equ e, 0xF5

.global a

.end