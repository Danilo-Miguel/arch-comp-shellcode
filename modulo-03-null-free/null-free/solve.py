from pwn import asm, context

context.arch = "amd64"

# O resultado nao pode conter b"\x00".
payload = asm("nop")
assert b"\x00" not in payload
open("/home/hacker/payload.bin", "wb").write(payload)