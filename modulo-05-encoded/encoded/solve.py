from pwn import asm, context

context.arch = "amd64"

# Gere primeiro o estagio write, codifique-o e acrescente o decoder PIC.
payload = asm("nop")
open("/home/hacker/payload.bin", "wb").write(payload)