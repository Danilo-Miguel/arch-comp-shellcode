from pwn import asm, context

context.arch = "amd64"

payload = asm("""
	mov eax, 1
	mov edi, 1
	mov edx, 17
	lea rsi, [rip + message]
	syscall
	mov eax, 60
	xor edi, edi
	syscall
message:
	.ascii "COMANDO_UNICO_OK\\n"
""")
open("/home/hacker/payload.bin", "wb").write(payload)