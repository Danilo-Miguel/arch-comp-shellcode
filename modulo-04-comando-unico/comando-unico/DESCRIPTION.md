# Execucao de comando unico

Em vez de abrir um shell, escreva shellcode que use a syscall `write` para emitir exatamente:

```
COMANDO_UNICO_OK
```

com uma quebra de linha ao final. O checker rejeita saida extra. Esse exercicio representa um payload com efeito definido, sem canal interativo.