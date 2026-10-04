from pwn import asm, context

# Fallback em pwntools: gera os mesmos bytes que o payload do msfvenom produziria,
# caso o container esteja sem rede para instalar o Metasploit. O caminho principal,
# com msfvenom, esta no DESCRIPTION.md.
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
