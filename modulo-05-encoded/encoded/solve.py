from pwn import asm, context

context.arch = "amd64"

key = 0xAA
stage2 = asm("""
	mov eax, 1
	mov edi, 1
	mov edx, 10
	lea rsi, [rip + message]
	syscall
	mov eax, 60
	xor edi, edi
	syscall
message:
	.ascii "ENCODED_OK"
""")
encoded = bytes(byte ^ key for byte in stage2)
encoded_text = ", ".join(f"0x{byte:02x}" for byte in encoded)

payload = asm(f"""
	jmp get_data
decoder:
	pop rsi
	mov rdi, rsi
	mov ecx, {len(encoded)}
decode:
	xor byte ptr [rsi], {key}
	inc rsi
	loop decode
	jmp rdi
get_data:
	call decoder
	.byte {encoded_text}
""")
assert b"ENCODED_OK" not in payload
open("/home/hacker/payload.bin", "wb").write(payload)