push 1    ;
pop rax   ;
next:     ;
push rax  ;<---+
push rax  ;    |
mul       ;    |
push rax  ;    |
push 1    ;    |
add       ;    |
pop rax   ;    |
add       ;    |
jmp next  ;----+
