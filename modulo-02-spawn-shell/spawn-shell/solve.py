from pwn import asm, context

context.arch = "amd64"

# Monte aqui um execve("/bin/sh", argv, NULL).
payload = asm("nop")
open("/home/hacker/payload.bin", "wb").write(payload)