# Metasploit: framework e primeiro payload

Nos modulos 1 a 7 voce montou shellcode "na mao" com Assembly + `pwntools`. Agora
voce vai conhecer o **Metasploit Framework**, que automatiza grande parte desse
trabalho - e entender onde ele se encaixa no que voce ja sabe.

> **Importante - ambiente deste dojo:** o container do pwn.college e **offline e sem
> `msfvenom`** (nao ha rede nem o pacote instalado, e `sudo`/`apt` nao funcionam no
> workspace padrao). Por isso, **aqui a pontuacao sai pelo `pwntools`**, que ja esta
> instalado e funciona sem internet. O `msfvenom` entra como **conceito e
> demonstracao** - voce o roda fora do pwn.college (ver secao no fim). O
> `/challenge/check` valida o **comportamento** do payload, nao a ferramenta.

## O que e o Metasploit

O Metasploit Framework e um conjunto de ferramentas para desenvolvimento e teste de
exploits em laboratorio autorizado. As pecas que interessam para nos, e o paralelo
com o que voce ja fez:

| Metasploit | Equivalente no dojo |
|---|---|
| **payload** (codigo que roda no alvo) | o shellcode dos modulos 1-7 |
| **`msfvenom`** (gera payloads por linha de comando) | o seu `pwntools` |
| **encoders** (transformam os bytes) | o decoder XOR do modulo 5 (`encoded`) |
| **stagers / stages** (payload em etapas) | o payload staged do modulo 6 |
| **`msfconsole`** (orquestra modulos de exploit) | - (so conceito aqui) |

A ideia central: tudo que voce fez manualmente tem um equivalente no framework.

## O que voce precisa fazer (caminho que pontua: pwntools)

Gere um payload que produza a string exata `MSF_SETUP_OK` e grave os bytes crus em
`/home/hacker/payload.bin`. Crie `/home/hacker/solve.py`:

```python
from pwn import asm, context

context.arch = "amd64"
payload = asm("""
    mov eax, 1
    mov edi, 1
    lea rsi, [rip + message]
    mov edx, 13
    syscall
    mov eax, 60
    xor edi, edi
    syscall
message:
    .ascii "MSF_SETUP_OK\\n"
""")
open("/home/hacker/payload.bin", "wb").write(payload)
```

Esse shellcode faz `write(1, "MSF_SETUP_OK\n", 13)` e depois `exit(0)` - o mesmo
efeito do payload `linux/x64/exec` do `msfvenom`, so que montado com `pwntools`.

## Fluxo completo

```bash
python3 /home/hacker/solve.py
od -An -tx1 -v /home/hacker/payload.bin   # veja os bytes gerados
/challenge/check
```

O checker compila o harness C, executa `payload.bin` em memoria executavel e procura
`MSF_SETUP_OK` na saida.

## Demonstracao do msfvenom (fora do pwn.college)

Para ver a ferramenta real, rode em um ambiente **com internet e root** - sua
maquina, uma VM Kali, ou Docker:

```bash
# exemplo com Docker, fora do pwn.college
docker run -it metasploitframework/metasploit-framework
# ja dentro do container do metasploit:
msfvenom -p linux/x64/exec CMD='/bin/echo MSF_SETUP_OK' -f raw -o payload.bin
```

- `-p linux/x64/exec` escolhe o payload que executa um comando;
- `CMD=...` e o comando (aqui, imprimir a evidencia);
- `-f raw` pede bytes crus, sem cabecalho ELF (igual ao seu `payload.bin`);
- `-o` grava no arquivo.

Compare os bytes do `msfvenom` com os do `pwntools` (`od -An -tx1 -v`): sao
representacoes diferentes do mesmo tipo de efeito. E essa a licao do modulo.
