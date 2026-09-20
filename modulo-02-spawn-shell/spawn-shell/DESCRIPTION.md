# Spawn shell local

## O que voce precisa fazer

Crie `/home/hacker/solve.py` e use `pwn.asm` para montar shellcode x86-64 que execute `/bin/sh` via `execve`. Grave o resultado em `/home/hacker/payload.bin`.

O Assembly deve ficar dentro do Python. Nao crie um ELF nem um `.asm` separado:

```python
from pwn import asm, context

context.arch = "amd64"
payload = asm("""
	# escreva aqui o shellcode execve("/bin/sh", argv, NULL)
""")
open("/home/hacker/payload.bin", "wb").write(payload)
```

## Teste

```bash
python3 /home/hacker/solve.py
/challenge/check
```

O checker inicia o harness C, envia `printf SHELL_OK; exit` pela entrada padrao e procura `SHELL_OK` na saida. O shell roda como `hacker` em um container isolado.