from pwn import asm, context

context.arch = "amd64"
payload = asm("""
    mov eax, 60
    mov edi, 42
    syscall
""")
open("/home/hacker/payload.bin", "wb").write(payload)
