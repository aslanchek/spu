; Discriminant of a quadratic equation
;
; compile with:
;     $ ./asm program.asm a.exe
;
; run with:
;     $ ./spu a.exe

in       ;a
in       ;b
in       ;c

pop rcx
pop rbx
pop rax

push rbx
push rbx
mul      ;b^2
push 4
push rax
push rcx
mul      ;ac
mul      ;4ac
sub      ;b^2 - 4ac
out      ;print

hlt
