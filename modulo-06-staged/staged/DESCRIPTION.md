# Shellcode staged

## O que voce precisa fazer

Crie `/home/hacker/solve.py`. Dentro dele, use `pwn.asm` para gerar dois conjuntos de bytes e grave dois arquivos:

- `/home/hacker/payload.bin`: stage 1, com no maximo 64 bytes;
- `/home/hacker/stage2.bin`: stage 2, com no maximo 512 bytes.

O modelo de escrita e:

```python
from pwn import asm, context

context.arch = "amd64"
stage1 = asm("...")
stage2 = asm("...")
open("/home/hacker/payload.bin", "wb").write(stage1)
open("/home/hacker/stage2.bin", "wb").write(stage2)
```

Nao crie um ELF ou um `.asm` separado. Os dois arquivos sao bytes crus.

O checker compila o harness C e envia `stage2.bin` pela entrada padrao para o stage 1. O stage 1 deve ler esses bytes para memoria executavel e transferir o controle para eles. O stage 2 deve escrever `STAGED_OK` e encerrar. Isso modela entrega em dois estagios sem usar rede.

Teste:

```bash
python3 /home/hacker/solve.py
ls -l /home/hacker/payload.bin /home/hacker/stage2.bin
/challenge/check
```