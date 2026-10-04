from pwn import asm, context

# Gabarito do modulo 9 (fallback pwntools, sem 0x00).
# Caminho com a ferramenta real:
#   msfvenom -p linux/x64/exec CMD='/bin/echo MSF_ENC_OK' -b '\x00' -f raw -o /home/hacker/payload.bin
context.arch = "amd64"

payload = asm("""
    movabs rax, 0x4141414141414b4f
    push rax
    movabs rax, 0x5f434e455f46534d
    push rax
    mov rsi, rsp
    push 1
    pop rdi
    push 10
    pop rdx
    push 1
    pop rax
    syscall
    push 60
    pop rax
    xor edi, edi
    syscall
""")
assert b"\x00" not in payload
open("/home/hacker/payload.bin", "wb").write(payload)
