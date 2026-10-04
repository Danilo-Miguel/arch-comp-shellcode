from pwn import asm, context

# Fallback pwntools equivalente ao payload do msfvenom, sem nenhum byte 0x00.
# A mensagem "MSF_ENC_OK" e montada na pilha em runtime (sem dados literais no
# codigo) e os registradores sao carregados com push/pop para evitar os zeros
# que um "mov edi, 1" geraria. Caminho real no DESCRIPTION.md (msfvenom -b '\x00').
context.arch = "amd64"

payload = asm("""
    movabs rax, 0x4141414141414b4f   /* "OK" + preenchimento 0x41 (nao usado no write) */
    push rax
    movabs rax, 0x5f434e455f46534d   /* "MSF_ENC_" em little-endian */
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
assert b"\x00" not in payload, "o payload nao pode conter 0x00"
open("/home/hacker/payload.bin", "wb").write(payload)
