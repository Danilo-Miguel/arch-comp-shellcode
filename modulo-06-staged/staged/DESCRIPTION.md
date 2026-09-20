# Shellcode staged

Crie dois arquivos:

- `/home/hacker/payload.bin`: stage 1, com no maximo 64 bytes;
- `/home/hacker/stage2.bin`: stage 2, com no maximo 512 bytes.

O checker envia `stage2.bin` pela entrada padrao. O stage 1 deve ler esses bytes para memoria executavel e transferir o controle para eles. O stage 2 deve escrever `STAGED_OK` e encerrar. Isso modela entrega em dois estagios sem usar rede.