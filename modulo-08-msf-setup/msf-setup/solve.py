from pwn import asm, context

# Caminho principal no pwn.college (offline, sem msfvenom): gera com pwntools os
# mesmos bytes que o payload linux/x64/exec do msfvenom produziria. A demonstracao
# do msfvenom (fora do pwn.college) esta no DESCRIPTION.md.
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
