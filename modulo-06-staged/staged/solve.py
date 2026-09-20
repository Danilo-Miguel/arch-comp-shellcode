from pwn import asm, context

context.arch = "amd64"

# O stage 1 deve ler stdin e saltar para os bytes recebidos.
stage1 = asm("nop")
# O stage 2 deve escrever STAGED_OK\n.
stage2 = asm("nop")
open("/home/hacker/payload.bin", "wb").write(stage1)
open("/home/hacker/stage2.bin", "wb").write(stage2)