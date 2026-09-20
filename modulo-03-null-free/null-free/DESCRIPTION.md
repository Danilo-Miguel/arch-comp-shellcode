# Shellcode null-free

Repita o desafio de spawn shell, mas agora `payload.bin` nao pode conter o byte `0x00`.

O objetivo e aplicar a troca discutida no roteiro: montar os registradores sem instrucoes que criem zero-padding literal. O checker primeiro rejeita bytes nulos e depois confirma que o shell executa `printf NULL_FREE_OK`.