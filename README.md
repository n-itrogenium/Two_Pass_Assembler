# Dvoprolazni asembler
Cilj ovog zadatka jeste realizacija dvoprolaznog asemblera za apstraktni računarski sistem. Ulaz asemblera je tekstualna datoteka sa izvornim asemblerskim kodom napisanim u
skladu sa odgovarajućom sintaksom, dok je izlaz asemblera tekstualna datoteka koja
predstavlja predmetni program baziran na školskoj varijanti ELF formata.


## Prevođenje i pokretanje projekta iz terminala
Prevođenje projekta se vrši zadavanjem sledeće komande:
```
g++ -o asembler src/assembler.cpp src/main.cpp src/operations.cpp src/section.cpp src/symboltable.cpp -I inc
```
Jednim pokretanjem `asembler` vrši asembliranje jedne ulazne datoteke. Naziv ulazne datoteke sa izvornim asemblerskim kodom zadaje se kao samostalni argument komandne linije. Način
pokretanja `asembler` jeste sledeći:
```
./asembler -o <naziv_izlazne_datoteke> <naziv_ulazne_datoteke>
```
Opcija komandne linije `-o` postavlja svoj parametar `<naziv_izlazne_datoteke>` za naziv izlazne datoteke koja predstavlja rezultat asembliranja.


## Rezultati asembliranja
U nastavku su date ulazne test datoteke koje koriste različite direktive i instrukcije zajedno sa svojim rezultujućim datotekama nakon asembliranja, koje sadrže tabelu simbola, relokacione zapise i sadržaje sekcija.
##### interrupts.s
```
# file: interrupts.s
.section ivt
  .word isr_reset
  .skip 2 
  .word isr_timer
  .word isr_terminal
  .skip 8
.extern myStart, myCounter
.section isr
.equ term_out, 0xFF00
.equ term_in, 0xFF02
.equ asciiCode, 84 
# prekidna rutina za reset
isr_reset:
  jmp myStart
# prekidna rutina za tajmer
isr_timer:
  push r0
  ldr r0, $asciiCode
  str r0, term_out
  pop r0
  iret
# prekidna rutina za terminal
isr_terminal:
  push r0
  push r1
  ldr r0, term_in
  str r0, term_out
  ldr r0, %myCounter 
  ldr r1, $1
  add r0, r1
  str r0, myCounter 
  pop r1
  pop r0
  iret
.end
```

##### interrupts.o
```
============================SYMBOL TABLE==============================
          Label        Section     Value      Size     Scope   Ordinal
----------------------------------------------------------------------
            isr            isr         0        62         L         1
            ivt            ivt         0        16         L         2
      asciiCode            abs        54                   L         3
      isr_reset            isr         0                   L         4
   isr_terminal            isr        16                   L         5
      isr_timer            isr         5                   L         6
      myCounter              ?         0                   G         7
        myStart              ?         0                   G         8
        term_in            abs      ff02                   L         9
       term_out            abs      ff00                   L        10



===========================RELOCATION TABLE===========================
Section: isr
    Offset   Rel. type   Ordinal
--------------------------------
         3         ABS         8
        41      PC_REL         7
        53         ABS         7



Section: ivt
    Offset   Rel. type   Ordinal
--------------------------------
         0         ABS         1
         4         ABS         1
         6         ABS         1



===============================SECTIONS===============================
Section: isr	[62 bytes]
-----------------------------------------------
50 f0 00 00 00 b0 60 12 a0 00 00 00 54 b0 00 04
ff 00 a0 06 42 20 b0 60 12 b0 61 12 a0 00 04 ff
02 b0 00 04 ff 00 a0 07 03 ff fe a0 10 00 00 01
70 01 b0 00 04 00 00 a0 16 42 a0 06 42 20 

Section: ivt	[16 bytes]
-----------------------------------------------
00 00 00 00 05 00 16 00 00 00 00 00 00 00 00 00
```

##### main.s
```
# file: main.s
.global myStart
.global myCounter
.section myCode
.equ tim_cfg, 0xFF10
myStart:
  ldr r0, $0x1
  str r0, tim_cfg
wait:
  ldr r1, myCounter
  ldr r1, $5
  cmp r0, r1
  jne wait
  halt
.section myData
myCounter:
  .word 0
.end
```

##### main.o
```
============================SYMBOL TABLE==============================
          Label        Section     Value      Size     Scope   Ordinal
----------------------------------------------------------------------
         myCode         myCode         0        28         L         1
         myData         myData         0         2         L         2
      myCounter         myData         0                   G         3
        myStart         myCode         0                   G         4
        tim_cfg            abs      ff10                   L         5
           wait         myCode         a                   L         6



===========================RELOCATION TABLE===========================
Section: myCode
    Offset   Rel. type   Ordinal
--------------------------------
        13         ABS         2
        25         ABS         1



===============================SECTIONS===============================
Section: myCode	[28 bytes]
-----------------------------------------------
a0 00 00 00 01 b0 00 04 ff 10 a0 10 04 00 00 a0
10 00 00 05 74 01 52 f0 00 00 0a 00 

Section: myData	[2 bytes]
-----------------------------------------------
00 00 
```


