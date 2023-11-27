push 1
pop rax
next:
push rax
push rax
mul
out
pop rbx
push rax
push 1
add
pop rax
push rax
push 11
jb next
hlt
