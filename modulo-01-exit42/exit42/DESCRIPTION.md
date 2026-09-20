# Shellcode minimo: `exit(42)`

## O que voce precisa fazer

Crie `/home/hacker/solve.py`. Dentro desse arquivo, o Assembly Linux x86-64 aparece como texto dentro de uma string Python. O `pwntools` monta esse texto e o transforma nos bytes do shellcode.

Voce nao precisa criar um arquivo `.asm` separado e nao precisa compilar um ELF. O Assembly fica dentro do Python; `asm()` transforma o texto em bytes; `open(..., "wb")` grava esses bytes em `payload.bin`.

Para este desafio, o arquivo completo `/home/hacker/solve.py` e:

```python
from pwn import asm, context

context.arch = "amd64"
payload = asm("""
	mov eax, 60
	mov edi, 42
	syscall
""")
open("/home/hacker/payload.bin", "wb").write(payload)
```

As tres linhas entre `asm("""` e `""")` sao Assembly, nao Python:

- `mov eax, 60` seleciona a syscall Linux `exit`;
- `mov edi, 42` define o codigo de saida;
- `syscall` executa a chamada.

O restante e Python: `asm()` monta o Assembly e `open(..., "wb")` grava o resultado no arquivo binario.

## Fluxo completo

```bash
python3 /home/hacker/solve.py
ls -l /home/hacker/payload.bin
/challenge/check
```

O primeiro comando executa o arquivo Python e gera `/home/hacker/payload.bin`. O segundo apenas confirma que o arquivo existe. Voce nao precisa abrir, copiar ou colar os bytes do `.bin` em outro lugar: o checker le esse arquivo diretamente.

O seu Python nao executa o shellcode. Ele apenas gera o arquivo binario. O `/challenge/check` compila e executa o harness C, que carrega `payload.bin` em memoria executavel e pula para os bytes. A flag aparece somente se o processo terminar com codigo `42`.

`payload.bin` e um arquivo de bytes crus: nao e um programa ELF e nao possui cabecalho.