# Spawn shell local

Gere `/home/hacker/payload.bin` com shellcode x86-64 que execute `/bin/sh` via `execve`.

O checker envia `printf SHELL_OK` para a entrada padrao do processo. A flag aparece quando essa mensagem volta pela saida padrao. O processo roda como `hacker` em um container isolado.

Use `pwn.asm` para montar os bytes puros e rode `/challenge/check`.