# Shellcode null-free

## O que voce precisa fazer

Crie `/home/hacker/solve.py` com um shellcode que execute `/bin/sh`, como no modulo anterior. A diferenca e que o arquivo final `/home/hacker/payload.bin` nao pode conter nenhum byte `0x00`.

```python
from pwn import asm, context

context.arch = "amd64"
payload = asm("""
	# execve("/bin/sh", argv, NULL), sem bytes nulos no resultado
""")
assert b"\x00" not in payload
open("/home/hacker/payload.bin", "wb").write(payload)
```

Execute:

```bash
python3 /home/hacker/solve.py
/challenge/check
```

O checker primeiro inspeciona os bytes do arquivo. Se nao houver `0x00`, ele executa o harness C e envia `printf NULL_FREE_OK; exit` ao shell.