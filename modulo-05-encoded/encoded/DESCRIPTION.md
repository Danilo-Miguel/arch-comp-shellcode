# Shellcode codificado

Crie um `payload.bin` com um pequeno decoder XOR seguido de um estagio codificado. Em runtime, o decoder deve recuperar e executar um estagio que escreva:

```
ENCODED_OK
```

O texto `ENCODED_OK` nao pode aparecer em claro dentro de `payload.bin`; o checker confirma a restricao antes de executar os bytes.