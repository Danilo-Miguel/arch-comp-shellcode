# Shellcode minimo: `exit(42)`

Crie `/home/hacker/payload.bin` contendo shellcode Linux x86-64 que termine o processo com codigo de saida `42`.

Use `pwn.asm`, e nao um ELF completo. O checker executa os bytes e verifica o codigo de saida.

```python
from pwn import asm, context

context.arch = "amd64"
payload = asm("...")
open("/home/hacker/payload.bin", "wb").write(payload)
```

Confira com:

```bash
/challenge/check
```