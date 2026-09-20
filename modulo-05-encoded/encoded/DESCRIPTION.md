# Shellcode codificado

## O que voce precisa fazer

Crie `/home/hacker/solve.py`. Esse Python deve montar o shellcode com `pwn.asm` e gravar `/home/hacker/payload.bin`. O arquivo final deve conter um decoder XOR seguido de um estagio codificado. Em runtime, o decoder deve recuperar e executar um estagio que escreva:

```
ENCODED_OK
```

O texto `ENCODED_OK` nao pode aparecer em claro dentro de `payload.bin`; o checker confirma a restricao antes de executar os bytes.

Fluxo:

```bash
python3 /home/hacker/solve.py
/challenge/check
```

O Python apenas monta e grava os bytes. O harness C executa o decoder; o proprio decoder deve alterar os bytes codificados em memoria e saltar para o estagio recuperado.