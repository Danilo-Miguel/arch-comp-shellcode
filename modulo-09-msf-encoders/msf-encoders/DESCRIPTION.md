# msfvenom: encoders, formatos e bad chars

Este modulo liga as ideias do `msfvenom` ao que voce viu no modulo 3 (`null-free`) e
no modulo 5 (`encoded`). O objetivo e gerar um payload que:

1. **nao contenha nenhum byte `0x00`** (bad char), e
2. produza a string exata `MSF_ENC_OK`.

> **Ambiente deste dojo:** como no modulo 8, o pwn.college e **offline e sem
> `msfvenom`**, entao **a pontuacao sai pelo `pwntools`** (secao principal abaixo). O
> `msfvenom` aparece como conceito e demonstracao externa no fim.

## Conceitos

- **Formato (`-f`)**: como o payload sai. `raw` = bytes crus (o que o harness le).
  Outros formatos (`-f c`, `-f python`, `-f hex`) sao so representacoes para colar em
  codigo; nao mudam os bytes executados.
- **Bad chars (`-b`)**: bytes que o canal de entrega corromperia. `0x00` termina
  strings em C (modulo 3); `0x0a` pode ser tratado como fim de linha. No `msfvenom`,
  `-b '\x00'` diz para evitar esses bytes; no `pwntools`, voce escolhe instrucoes que
  nao geram zeros.
- **Encoder (`-e`)**: transforma os bytes e adiciona um decoder que desfaz a
  transformacao em runtime (exatamente a ideia do modulo 5). Encoders servem para
  evitar bad chars, **nao** para "ficar indetectavel".

## O que voce precisa fazer (caminho que pontua: pwntools)

Monte um shellcode que imprime `MSF_ENC_OK` **sem nenhum byte `0x00`**. As tecnicas
sao as do modulo 3: montar a string na pilha com `movabs`/`push` e carregar os
registradores com `push`/`pop` (em vez de `mov edi, 1`, que gera zeros). Crie
`/home/hacker/solve.py`:

```python
from pwn import asm, context

context.arch = "amd64"
payload = asm("""
    movabs rax, 0x4141414141414b4f   /* "OK" + preenchimento 0x41 (nao usado no write) */
    push rax
    movabs rax, 0x5f434e455f46534d   /* "MSF_ENC_" em little-endian */
    push rax
    mov rsi, rsp
    push 1
    pop rdi
    push 10
    pop rdx
    push 1
    pop rax
    syscall
    push 60
    pop rax
    xor edi, edi
    syscall
""")
assert b"\x00" not in payload, "o payload nao pode conter 0x00"
open("/home/hacker/payload.bin", "wb").write(payload)
```

## Fluxo completo

```bash
python3 /home/hacker/solve.py
od -An -tx1 -v /home/hacker/payload.bin | grep -w 00   # nao deve achar nada
/challenge/check
```

O checker primeiro rejeita qualquer `0x00` no arquivo, depois executa o payload e
procura `MSF_ENC_OK`.

## Demonstracao do msfvenom (fora do pwn.college)

Em um ambiente com internet e root (sua maquina, Kali, Docker), o mesmo resultado
sem `0x00` sairia com um encoder:

```bash
msfvenom -p linux/x64/exec CMD='/bin/echo MSF_ENC_OK' \
         -b '\x00' -f raw -o payload.bin
# forcando um encoder explicito para ver o decoder:
msfvenom -p linux/x64/exec CMD='/bin/echo MSF_ENC_OK' \
         -e x64/xor_dynamic -b '\x00' -f raw -o payload.bin
```

O `-b '\x00'` faz o `msfvenom` escolher uma codificacao sem bytes nulos - a mesma
preocupacao que voce resolveu a mao no `pwntools`.
