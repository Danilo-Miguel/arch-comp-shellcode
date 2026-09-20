from pwn import asm, context

context.arch = "amd64"

# Reutilize a ideia de execve do modulo 2, agora entendendo o canal herdado.
payload = asm("nop")
open("/home/hacker/payload.bin", "wb").write(payload)