from pwn import asm, context

# Gabarito do modulo 8 (fallback pwntools).
# Caminho com a ferramenta real:
#   msfvenom -p linux/x64/exec CMD='/bin/echo MSF_SETUP_OK' -f raw -o /home/hacker/payload.bin
context.arch = "amd64"

payload = asm("""
    mov eax, 1
    mov edi, 1
    lea rsi, [rip + message]
    mov edx, 13
    syscall
    mov eax, 60
    xor edi, edi
    syscall
message:
    .ascii "MSF_SETUP_OK\\n"
""")
open("/home/hacker/payload.bin", "wb").write(payload)
