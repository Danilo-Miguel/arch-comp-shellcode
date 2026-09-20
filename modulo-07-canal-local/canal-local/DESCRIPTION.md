# Canal local controlado

## O que voce precisa fazer

Crie `/home/hacker/solve.py` e gere `/home/hacker/payload.bin` com `pwn.asm`. O shellcode deve executar `/bin/sh` preservando stdin e stdout:

```python
from pwn import asm, context

context.arch = "amd64"
payload = asm("""
	# execve("/bin/sh", argv, NULL)
""")
open("/home/hacker/payload.bin", "wb").write(payload)
```

Este desafio representa o objetivo de um shell conectado sem criar conexoes de rede: stdin e stdout ja formam um canal local controlado pelo checker.

Execute:

```bash
python3 /home/hacker/solve.py
/challenge/check
```

O checker envia `printf CANAL_LOCAL_OK; exit` e espera a resposta. Compare essa entrega com a de um reverse ou bind shell: aqui o transporte ja esta estabelecido, portanto nao ha `socket`, `connect`, `bind` ou `listen`.