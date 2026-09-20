from pwn import asm, context

context.arch = "amd64"

# Escreva COMANDO_UNICO_OK\n no descritor 1 usando write.
payload = asm("nop")
open("/home/hacker/payload.bin", "wb").write(payload)