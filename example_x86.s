.intel_syntax_noprefix

.globl fib_rec
fib_rec:
 push rbp
 mov rbp, rsp
 sub rsp, 512
 mov [rbp-8], rdi
 mov rax, rbp
 sub rax, 8
 push rax
 pop rax
 mov rax, [rax]
 push rax
 push 1
 pop rdi
 pop rax
 cmp rdi, rax
 setge al
 movzx rax, al
 push rax
 pop rax
 cmp rax, 0
 je .L.B1_0
 mov rax, rbp
 sub rax, 8
 push rax
 pop rax
 mov rax, [rax]
 push rax
 pop rax
 jmp .L.ret.fib_rec
 push rax
 pop rax
.L.B1_0:
 pop rax
 mov rax, rbp
 sub rax, 8
 push rax
 pop rax
 mov rax, [rax]
 push rax
 push 1
 pop rdi
 pop rax
 sub rax, rdi
 push rax
 pop rdi
 mov rax, rsp
 and rax, 15
 jnz .L.call.3
 mov rax, 0
 call fib_rec
 jmp .L.end.3
.L.call.3:
 sub rsp, 8
 mov rax, 0
 call fib_rec
 add rsp, 8
.L.end.3:
 push rax
 mov rax, rbp
 sub rax, 8
 push rax
 pop rax
 mov rax, [rax]
 push rax
 push 2
 pop rdi
 pop rax
 sub rax, rdi
 push rax
 pop rdi
 mov rax, rsp
 and rax, 15
 jnz .L.call.4
 mov rax, 0
 call fib_rec
 jmp .L.end.4
.L.call.4:
 sub rsp, 8
 mov rax, 0
 call fib_rec
 add rsp, 8
.L.end.4:
 push rax
 pop rdi
 pop rax
 add rax, rdi
 push rax
 pop rax
 jmp .L.ret.fib_rec
 push rax
 pop rax
 push rax
 pop rax
.L.ret.fib_rec:
 mov rsp, rbp
 pop rbp
 ret

.globl fib_loop
fib_loop:
 push rbp
 mov rbp, rsp
 sub rsp, 512
 mov [rbp-8], rdi
 mov rax, rbp
 sub rax, 8
 push rax
 pop rax
 mov rax, [rax]
 push rax
 push 0
 pop rdi
 pop rax
 cmp rdi, rax
 sete al
 movzx rax, al
 push rax
 pop rax
 cmp rax, 0
 je .L.B1_5
 push 0
 pop rax
 jmp .L.ret.fib_loop
 push rax
 pop rax
.L.B1_5:
 pop rax
 mov rax, rbp
 sub rax, 16
 push rax
 push 0
 pop rdi
 pop rax
 mov [rax], rdi
 push rdi
 pop rax
 mov rax, rbp
 sub rax, 24
 push rax
 push 1
 pop rdi
 pop rax
 mov [rax], rdi
 push rdi
 pop rax
 mov rax, rbp
 sub rax, 32
 push rax
 push 1
 pop rdi
 pop rax
 mov [rax], rdi
 push rdi
 pop rax
.L.B0_7:
 mov rax, rbp
 sub rax, 32
 push rax
 pop rax
 mov rax, [rax]
 push rax
 mov rax, rbp
 sub rax, 8
 push rax
 pop rax
 mov rax, [rax]
 push rax
 pop rdi
 pop rax
 cmp rdi, rax
 setg al
 movzx rax, al
 push rax
 pop rax
 cmp rax, 0
 je .L.B1_7
 mov rax, rbp
 sub rax, 24
 push rax
 mov rax, rbp
 sub rax, 16
 push rax
 pop rax
 mov rax, [rax]
 push rax
 mov rax, rbp
 sub rax, 24
 push rax
 pop rax
 mov rax, [rax]
 push rax
 pop rdi
 pop rax
 add rax, rdi
 push rax
 pop rdi
 pop rax
 mov [rax], rdi
 push rdi
 pop rax
 mov rax, rbp
 sub rax, 16
 push rax
 mov rax, rbp
 sub rax, 24
 push rax
 pop rax
 mov rax, [rax]
 push rax
 mov rax, rbp
 sub rax, 16
 push rax
 pop rax
 mov rax, [rax]
 push rax
 pop rdi
 pop rax
 sub rax, rdi
 push rax
 pop rdi
 pop rax
 mov [rax], rdi
 push rdi
 pop rax
 push rax
 pop rax
 mov rax, rbp
 sub rax, 32
 push rax
 mov rax, rbp
 sub rax, 32
 push rax
 pop rax
 mov rax, [rax]
 push rax
 push 1
 pop rdi
 pop rax
 add rax, rdi
 push rax
 pop rdi
 pop rax
 mov [rax], rdi
 push rdi
 pop rax
 jmp .L.B0_7
.L.B1_7:
 push rax
 pop rax
 mov rax, rbp
 sub rax, 24
 push rax
 pop rax
 mov rax, [rax]
 push rax
 pop rax
 jmp .L.ret.fib_loop
 push rax
 pop rax
 push rax
 pop rax
.L.ret.fib_loop:
 mov rsp, rbp
 pop rbp
 ret

.globl print_num
print_num:
 push rbp
 mov rbp, rsp
 sub rsp, 512
 mov [rbp-8], rdi
 push 0
 mov rax, rbp
 sub rax, 8
 push rax
 pop rax
 mov rax, [rax]
 push rax
 pop rdi
 pop rax
 cmp rdi, rax
 setg al
 movzx rax, al
 push rax
 pop rax
 cmp rax, 0
 je .L.B1_9
 mov rax, rbp
 sub rax, 8
 push rax
 pop rax
 mov rax, [rax]
 push rax
 push 10
 pop rdi
 pop rax
 cqo
 idiv rdi
 push rax
 pop rdi
 mov rax, rsp
 and rax, 15
 jnz .L.call.10
 mov rax, 0
 call print_num
 jmp .L.end.10
.L.call.10:
 sub rsp, 8
 mov rax, 0
 call print_num
 add rsp, 8
.L.end.10:
 push rax
 pop rax
 mov rax, rbp
 sub rax, 8
 push rax
 pop rax
 mov rax, [rax]
 push rax
 push 10
 pop rdi
 pop rax
 cqo
 idiv rdi
 mov rax, rdx
 push rax
 push 48
 pop rdi
 pop rax
 add rax, rdi
 push rax
 pop rdi
 mov rax, rsp
 and rax, 15
 jnz .L.call.11
 mov rax, 0
 call _putchar
 jmp .L.end.11
.L.call.11:
 sub rsp, 8
 mov rax, 0
 call _putchar
 add rsp, 8
.L.end.11:
 push rax
 pop rax
 push rax
 pop rax
.L.B1_9:
 pop rax
 push rax
 pop rax
.L.ret.print_num:
 mov rsp, rbp
 pop rbp
 ret

.globl print_hello_world
print_hello_world:
 push rbp
 mov rbp, rsp
 sub rsp, 512
 push 72
 pop rdi
 mov rax, rsp
 and rax, 15
 jnz .L.call.12
 mov rax, 0
 call _putchar
 jmp .L.end.12
.L.call.12:
 sub rsp, 8
 mov rax, 0
 call _putchar
 add rsp, 8
.L.end.12:
 push rax
 pop rax
 push 101
 pop rdi
 mov rax, rsp
 and rax, 15
 jnz .L.call.13
 mov rax, 0
 call _putchar
 jmp .L.end.13
.L.call.13:
 sub rsp, 8
 mov rax, 0
 call _putchar
 add rsp, 8
.L.end.13:
 push rax
 pop rax
 push 108
 pop rdi
 mov rax, rsp
 and rax, 15
 jnz .L.call.14
 mov rax, 0
 call _putchar
 jmp .L.end.14
.L.call.14:
 sub rsp, 8
 mov rax, 0
 call _putchar
 add rsp, 8
.L.end.14:
 push rax
 pop rax
 push 108
 pop rdi
 mov rax, rsp
 and rax, 15
 jnz .L.call.15
 mov rax, 0
 call _putchar
 jmp .L.end.15
.L.call.15:
 sub rsp, 8
 mov rax, 0
 call _putchar
 add rsp, 8
.L.end.15:
 push rax
 pop rax
 push 111
 pop rdi
 mov rax, rsp
 and rax, 15
 jnz .L.call.16
 mov rax, 0
 call _putchar
 jmp .L.end.16
.L.call.16:
 sub rsp, 8
 mov rax, 0
 call _putchar
 add rsp, 8
.L.end.16:
 push rax
 pop rax
 push 44
 pop rdi
 mov rax, rsp
 and rax, 15
 jnz .L.call.17
 mov rax, 0
 call _putchar
 jmp .L.end.17
.L.call.17:
 sub rsp, 8
 mov rax, 0
 call _putchar
 add rsp, 8
.L.end.17:
 push rax
 pop rax
 push 32
 pop rdi
 mov rax, rsp
 and rax, 15
 jnz .L.call.18
 mov rax, 0
 call _putchar
 jmp .L.end.18
.L.call.18:
 sub rsp, 8
 mov rax, 0
 call _putchar
 add rsp, 8
.L.end.18:
 push rax
 pop rax
 push 87
 pop rdi
 mov rax, rsp
 and rax, 15
 jnz .L.call.19
 mov rax, 0
 call _putchar
 jmp .L.end.19
.L.call.19:
 sub rsp, 8
 mov rax, 0
 call _putchar
 add rsp, 8
.L.end.19:
 push rax
 pop rax
 push 111
 pop rdi
 mov rax, rsp
 and rax, 15
 jnz .L.call.20
 mov rax, 0
 call _putchar
 jmp .L.end.20
.L.call.20:
 sub rsp, 8
 mov rax, 0
 call _putchar
 add rsp, 8
.L.end.20:
 push rax
 pop rax
 push 114
 pop rdi
 mov rax, rsp
 and rax, 15
 jnz .L.call.21
 mov rax, 0
 call _putchar
 jmp .L.end.21
.L.call.21:
 sub rsp, 8
 mov rax, 0
 call _putchar
 add rsp, 8
.L.end.21:
 push rax
 pop rax
 push 108
 pop rdi
 mov rax, rsp
 and rax, 15
 jnz .L.call.22
 mov rax, 0
 call _putchar
 jmp .L.end.22
.L.call.22:
 sub rsp, 8
 mov rax, 0
 call _putchar
 add rsp, 8
.L.end.22:
 push rax
 pop rax
 push 100
 pop rdi
 mov rax, rsp
 and rax, 15
 jnz .L.call.23
 mov rax, 0
 call _putchar
 jmp .L.end.23
.L.call.23:
 sub rsp, 8
 mov rax, 0
 call _putchar
 add rsp, 8
.L.end.23:
 push rax
 pop rax
 push 33
 pop rdi
 mov rax, rsp
 and rax, 15
 jnz .L.call.24
 mov rax, 0
 call _putchar
 jmp .L.end.24
.L.call.24:
 sub rsp, 8
 mov rax, 0
 call _putchar
 add rsp, 8
.L.end.24:
 push rax
 pop rax
 push rax
 pop rax
.L.ret.print_hello_world:
 mov rsp, rbp
 pop rbp
 ret

.globl _main
_main:
 push rbp
 mov rbp, rsp
 sub rsp, 512
 mov rax, rsp
 and rax, 15
 jnz .L.call.25
 mov rax, 0
 call print_hello_world
 jmp .L.end.25
.L.call.25:
 sub rsp, 8
 mov rax, 0
 call print_hello_world
 add rsp, 8
.L.end.25:
 push rax
 pop rax
 push 32
 pop rdi
 mov rax, rsp
 and rax, 15
 jnz .L.call.26
 mov rax, 0
 call _putchar
 jmp .L.end.26
.L.call.26:
 sub rsp, 8
 mov rax, 0
 call _putchar
 add rsp, 8
.L.end.26:
 push rax
 pop rax
 push 21
 pop rdi
 mov rax, rsp
 and rax, 15
 jnz .L.call.28
 mov rax, 0
 call fib_rec
 jmp .L.end.28
.L.call.28:
 sub rsp, 8
 mov rax, 0
 call fib_rec
 add rsp, 8
.L.end.28:
 push rax
 pop rdi
 mov rax, rsp
 and rax, 15
 jnz .L.call.27
 mov rax, 0
 call print_num
 jmp .L.end.27
.L.call.27:
 sub rsp, 8
 mov rax, 0
 call print_num
 add rsp, 8
.L.end.27:
 push rax
 pop rax
 push 32
 pop rdi
 mov rax, rsp
 and rax, 15
 jnz .L.call.29
 mov rax, 0
 call _putchar
 jmp .L.end.29
.L.call.29:
 sub rsp, 8
 mov rax, 0
 call _putchar
 add rsp, 8
.L.end.29:
 push rax
 pop rax
 push 21
 pop rdi
 mov rax, rsp
 and rax, 15
 jnz .L.call.31
 mov rax, 0
 call fib_loop
 jmp .L.end.31
.L.call.31:
 sub rsp, 8
 mov rax, 0
 call fib_loop
 add rsp, 8
.L.end.31:
 push rax
 pop rdi
 mov rax, rsp
 and rax, 15
 jnz .L.call.30
 mov rax, 0
 call print_num
 jmp .L.end.30
.L.call.30:
 sub rsp, 8
 mov rax, 0
 call print_num
 add rsp, 8
.L.end.30:
 push rax
 pop rax
 push 10
 pop rdi
 mov rax, rsp
 and rax, 15
 jnz .L.call.32
 mov rax, 0
 call _putchar
 jmp .L.end.32
.L.call.32:
 sub rsp, 8
 mov rax, 0
 call _putchar
 add rsp, 8
.L.end.32:
 push rax
 pop rax
 push 0
 pop rax
 jmp .L.ret.main
 push rax
 pop rax
 push rax
 pop rax
.L.ret.main:
 mov rsp, rbp
 pop rbp
 ret
