# Corrupcao de memoria: NOP sled

Ultima primitiva do bloco. Nos modulos anteriores o alvo do salto era um endereco
exato. Na vida real voce quase nunca acerta o byte exato onde o shellcode comeca -
o endereco varia um pouco a cada execucao. O **NOP sled** resolve isso.

## A ideia

`0x90` e a instrucao `nop` (nao faz nada). Se voce coloca uma longa rampa de `nop`
**antes** do shellcode, qualquer salto que caia em algum ponto da rampa vai apenas
"deslizar" executando `nop` ate chegar no shellcode. Voce troca precisao por
margem.

## O harness

`/challenge/run_sled.c` carrega seu payload em memoria executavel e pula para um
ponto **aleatorio** entre `memory+0` e `memory+199` (jitter de ate 200 bytes),
simulando a imprecisao. O checker roda 5 vezes; seu payload precisa funcionar em
todas.

## O que voce precisa fazer

Monte: `[ NOP sled >= 200 bytes ][ shellcode que imprime NOP_SLED_OK ]`.

```python
from pwn import asm, context

context.arch = "amd64"
shellcode = asm("""
    mov eax, 1
    mov edi, 1
    lea rsi, [rip + message]
    mov edx, 12
    syscall
    mov eax, 60
    xor edi, edi
    syscall
message:
    .ascii "NOP_SLED_OK\\n"
""")
sled = b"\\x90" * 256          # maior que o jitter de 200
payload = sled + shellcode
open("/home/hacker/payload.bin", "wb").write(payload)
```

Como o salto cai no maximo em `memory+199` e o sled tem 256 `nop`, a CPU sempre
alcanca o shellcode.

## Fluxo completo

```bash
python3 /home/hacker/solve.py
/challenge/check
```

## Por que isto importa

O NOP sled aumenta a confiabilidade do exploit quando o endereco de pouso e
incerto (por exemplo, com ASLR parcial ou pilha que varia). Ele prepara o terreno
para os modulos de buffer overflow, onde voce vai apontar o retorno para uma regiao
e quer margem de erro. Para a defesa, grandes sequencias de `0x90` em dados sao um
indicador classico que IDS/EDR procuram.
