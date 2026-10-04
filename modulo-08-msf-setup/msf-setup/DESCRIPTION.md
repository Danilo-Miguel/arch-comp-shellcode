# Metasploit: framework e primeiro payload

Nos modulos 1 a 7 voce montou shellcode "na mao" com Assembly + `pwntools`. Agora
voce vai conhecer o **Metasploit Framework**, que automatiza grande parte desse
trabalho. A ideia e a mesma do dojo inteiro: o `msfvenom` gera **bytes crus** de
shellcode e o harness C executa esses bytes. Nada de rede, nada de alvo remoto.

## O que e o Metasploit

O Metasploit Framework e um conjunto de ferramentas para desenvolvimento e teste
de exploits em laboratorio autorizado. As pecas que interessam para nos:

- **payloads** - o codigo que roda no alvo (equivale ao shellcode que voce escreveu);
- **`msfvenom`** - o gerador de payloads por linha de comando (equivale ao seu `pwntools`);
- **encoders** - transformam os bytes do payload (modulo 9; lembra o modulo 5 `encoded`);
- **stagers / stages** - payload dividido em etapas (lembra o modulo 6 `staged`);
- **`msfconsole`** - o console interativo que orquestra modulos de exploit (aqui so como conceito).

Repare no paralelo: tudo que voce fez manualmente tem um equivalente no framework.

## Instalacao (passo a passo, feita na hora)

> **Atencao - privilegios no pwn.college:** o workspace padrao e *nao-privilegiado*,
> entao `sudo` e `apt` nao funcionam (`sudo: workspace is not privileged`). Para
> instalar o Metasploit voce precisa reiniciar o desafio no **workspace privilegiado**
> (opcao de "Practice"/privileged na tela de start do challenge). Nele voce ja e root,
> sem `sudo`. Observacao: o modo privilegiado serve para pratica e **nao pontua** a
> flag - a pontuacao sai pelo fallback em `pwntools` (abaixo).

No workspace privilegiado, instale pela forma oficial (nao use `apt`: o pacote
geralmente nao esta nos repositorios):

```bash
curl -fsSL https://raw.githubusercontent.com/rapid7/metasploit-omnibus/master/config/templates/metasploit-framework-wrappers/msfupdate.erb -o /tmp/msfinstall
chmod 755 /tmp/msfinstall
/tmp/msfinstall
```

Confirme:

```bash
msfvenom --version
msfvenom -l payloads | grep linux/x64
```

> Se o container estiver sem acesso a rede (o instalador falha) ou voce quiser apenas
> pontuar, use o `solve.py` de fallback (secao abaixo): ele usa `pwntools`, que ja
> vem instalado, para gerar **os mesmos bytes**. O `/challenge/check` valida o
> comportamento do payload, nao a ferramenta.

## O que voce precisa fazer

Gere um payload que produza a string exata `MSF_SETUP_OK` e grave os bytes crus em
`/home/hacker/payload.bin`.

Caminho com msfvenom (payload `exec`, que roda um comando local):

```bash
msfvenom -p linux/x64/exec CMD='/bin/echo MSF_SETUP_OK' -f raw -o /home/hacker/payload.bin
```

- `-p linux/x64/exec` escolhe o payload que executa um comando;
- `CMD=...` e o comando a executar (aqui, imprimir a evidencia);
- `-f raw` pede bytes crus, sem cabecalho ELF (igual ao seu `payload.bin`);
- `-o` grava no arquivo que o checker le.

Caminho de fallback com pwntools (sempre funciona):

```python
from pwn import asm, context

context.arch = "amd64"
msg = b"MSF_SETUP_OK\n"
payload = asm(f"""
    mov eax, 1
    mov edi, 1
    lea rsi, [rip + message]
    mov edx, {len(msg)}
    syscall
    mov eax, 60
    xor edi, edi
    syscall
message:
    .ascii "MSF_SETUP_OK\\n"
""")
open("/home/hacker/payload.bin", "wb").write(payload)
```

## Fluxo completo

```bash
# opcao A: msfvenom
msfvenom -p linux/x64/exec CMD='/bin/echo MSF_SETUP_OK' -f raw -o /home/hacker/payload.bin
# opcao B: fallback
python3 /home/hacker/solve.py

od -An -tx1 -v /home/hacker/payload.bin   # veja os bytes gerados
/challenge/check
```

O checker compila o harness C, executa `payload.bin` em memoria executavel e
procura `MSF_SETUP_OK` na saida.
