# Shellcode minimo: `exit(42)`

## O que voce precisa fazer

Crie `/home/hacker/solve.py`. Dentro desse arquivo, escreva Assembly Linux x86-64 usando `pwntools` para montar um shellcode que termine o processo com codigo de saida `42`.

Voce nao precisa criar um arquivo `.asm` separado e nao precisa compilar um ELF. O Assembly fica dentro do Python; `asm()` transforma o texto em bytes; `open(..., "wb")` grava esses bytes em `payload.bin`.

Comece com este modelo e substitua `nop` pelas instrucoes da tarefa:

```python
from pwn import asm, context

context.arch = "amd64"
payload = asm("""
	nop
""")
open("/home/hacker/payload.bin", "wb").write(payload)
```

## Fluxo completo

```bash
python3 /home/hacker/solve.py
ls -l /home/hacker/payload.bin
/challenge/check
```

O seu Python nao executa o shellcode. Ele apenas gera o arquivo binario. O `/challenge/check` compila e executa o harness C, que carrega `payload.bin` em memoria executavel e pula para os bytes. A flag aparece somente se o processo terminar com codigo `42`.

`payload.bin` e um arquivo de bytes crus: nao e um programa ELF e nao possui cabecalho.