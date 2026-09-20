from pwn import asm, context

context.arch = "amd64"

# Substitua pelas instrucoes que fazem exit(42).
payload = asm("nop")
open("/home/hacker/payload.bin", "wb").write(payload)