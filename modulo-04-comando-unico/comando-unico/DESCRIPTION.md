# Execucao de comando unico

Crie `/home/hacker/solve.py`. Dentro dele, escreva Assembly em `pwn.asm` e grave os bytes gerados em `/home/hacker/payload.bin`:

```python
from pwn import asm, context

context.arch = "amd64"
payload = asm("""
	# use write(1, mensagem, tamanho)
""")
open("/home/hacker/payload.bin", "wb").write(payload)
```

Em vez de abrir um shell, o shellcode deve usar a syscall `write` para emitir exatamente:

```
COMANDO_UNICO_OK
```

com uma quebra de linha ao final. Teste com:

```bash
python3 /home/hacker/solve.py
/challenge/check
```

O checker executa os bytes pelo harness C e rejeita qualquer saida extra. Esse exercicio representa um payload com efeito definido, sem canal interativo.