from pwn import asm, context

# Gabarito do modulo 13.
context.arch = "amd64"

shellcode = asm("""
    mov eax, 1
    mov edi, 1
    lea rsi, [rip + message]
    mov edx, 12
    syscall
    mov eax, 60
    xor edi, edi
    syscall
message:
    .ascii "NOP_SLED_OK\\n"
""")
payload = b"\x90" * 256 + shellcode
open("/home/hacker/payload.bin", "wb").write(payload)
