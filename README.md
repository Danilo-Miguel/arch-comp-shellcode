# Shellcode com pwntools

Dojo privado para praticar os conceitos de `Aula9_Shellcode_Roteiro.md` no pwn.college.

A parte expositiva sobre Metasploit (o que e, comandos de `msfconsole` e `msfvenom`,
para mostrar em aula fora do pwn.college) esta em [METASPLOIT_INTRO.md](METASPLOIT_INTRO.md).

Para conduzir a aula, consulte [NOTAS_PROFESSOR.md](NOTAS_PROFESSOR.md). Esse arquivo explica o objetivo pedagogico, o metodo tecnico, o papel do atacante e do defensor e o motivo de cada challenge.

Os desafios executam somente bytes fornecidos pelo aluno dentro do container do desafio. O harness de execucao e escrito em C, seguindo o roteiro: ele abre `payload.bin`, reserva memoria executavel com `mmap`, carrega os bytes e um ponteiro de funcao transfere o controle para o shellcode. Os exercicios de comunicacao usam entrada e saida locais e controladas; nao criam listeners, nao fazem conexoes externas e nao alteram usuarios, defesas ou outros processos.

## Estrutura

| Modulo | Tema | Evidencia de sucesso |
|---|---|---|
| 1 | `exit(42)` | processo termina com codigo 42 |
| 2 | `execve("/bin/sh")` | shell recebe e executa um comando controlado |
| 3 | null-free | shell funcional sem `0x00` |
| 4 | comando unico | syscall `write` produz uma mensagem exata |
| 5 | encoded | decodificador em runtime libera a mensagem |
| 6 | staged | primeiro estagio recebe e executa o segundo via stdin |
| 7 | canal local | shell usa stdin/stdout como canal ja estabelecido |
| 8 | msf-setup | instalar Metasploit e gerar o primeiro payload com `msfvenom` |
| 9 | msf-encoders | encoder/formato/bad chars com `msfvenom` (sem `0x00`) |
| 10 | var-adjacente | estouro sobrescreve variavel vizinha |
| 11 | ponteiro-funcao | estouro sobrescreve ponteiro de funcao e desvia a chamada |
| 12 | endereco-retorno | estouro sobrescreve o endereco de retorno salvo (ret2win) |
| 13 | nop-sled | NOP sled tolera pouso impreciso |
| 14 | offset | descobrir o offset ate o retorno com `cyclic` |
| 15 | controle-rip | controlar o RIP e saltar para o buffer |
| 16 | shellcode-injetado | injetar shellcode com sled e abrir shell |
| 17 | msfvenom-no-buffer | entregar shellcode do `msfvenom` pelo overflow |

Os modulos 1 a 9 executam shellcode em um harness controlado que nao tem
vulnerabilidade. Os modulos 10 a 17 introduzem **corrupcao de memoria** e **buffer
overflow**: cada um traz um programa C deliberadamente vulneravel (`vuln.c`, incluido
na pasta do challenge) compilado com `-fno-stack-protector -no-pie` (e `-z execstack`
nos modulos 15 a 17). Toda a atividade e local no container; nao ha rede, listener
nem alvo remoto. O bloco 8-9 e Metasploit: o `msfvenom` substitui o `pwntools` como
gerador de payloads, e os desafios aceitam um fallback em `pwntools` quando a
ferramenta nao esta instalada, porque o checker valida o comportamento, nao a
ferramenta.

## Fluxo do aluno

1. Entre no desafio no pwn.college.
2. Crie `/home/hacker/solve.py`.
3. Dentro dele, escreva o Assembly pedido como texto dentro de `asm("""...")`.
4. Execute `python3 /home/hacker/solve.py`; o `pwntools` monta o Assembly e grava os bytes crus em `/home/hacker/payload.bin`.
5. Rode `/challenge/check`; o checker compila o harness C, executa o `.bin` e verifica o resultado.

O arquivo `payload.bin` deve conter bytes crus, sem cabecalho ELF. Voce nao precisa criar um arquivo `.asm` separado: o Assembly fica dentro do `solve.py`. No modulo staged, o `solve.py` tambem cria `stage2.bin`. Python aparece somente no lado do aluno, por causa do `pwntools`; o carregador e o harness sao C.

## Onde esta cada solucao completa

Cada pasta de challenge ja possui um `solve.py` completo. Na plataforma, copie o conteudo do arquivo correspondente para `/home/hacker/solve.py`:

| Challenge | Arquivo completo | Arquivo gerado |
|---|---|---|
| `exit42` | `modulo-01-exit42/exit42/solve.py` | `/home/hacker/payload.bin` |
| `spawn-shell` | `modulo-02-spawn-shell/spawn-shell/solve.py` | `/home/hacker/payload.bin` |
| `null-free` | `modulo-03-null-free/null-free/solve.py` | `/home/hacker/payload.bin` |
| `comando-unico` | `modulo-04-comando-unico/comando-unico/solve.py` | `/home/hacker/payload.bin` |
| `encoded` | `modulo-05-encoded/encoded/solve.py` | `/home/hacker/payload.bin` |
| `staged` | `modulo-06-staged/staged/solve.py` | `/home/hacker/payload.bin` e `/home/hacker/stage2.bin` |
| `canal-local` | `modulo-07-canal-local/canal-local/solve.py` | `/home/hacker/payload.bin` |
| `msf-setup` | `modulo-08-msf-setup/msf-setup/solve.py` | `/home/hacker/payload.bin` |
| `msf-encoders` | `modulo-09-msf-encoders/msf-encoders/solve.py` | `/home/hacker/payload.bin` |
| `var-adjacente` | `modulo-10-var-adjacente/var-adjacente/solve.py` | `/home/hacker/payload.bin` |
| `ponteiro-funcao` | `modulo-11-ponteiro-funcao/ponteiro-funcao/solve.py` | `/home/hacker/payload.bin` |
| `endereco-retorno` | `modulo-12-endereco-retorno/endereco-retorno/solve.py` | `/home/hacker/payload.bin` |
| `nop-sled` | `modulo-13-nop-sled/nop-sled/solve.py` | `/home/hacker/payload.bin` |
| `offset` | `modulo-14-offset/offset/solve.py` | `/home/hacker/payload.bin` |
| `controle-rip` | `modulo-15-controle-rip/controle-rip/solve.py` | `/home/hacker/payload.bin` |
| `shellcode-injetado` | `modulo-16-shellcode-injetado/shellcode-injetado/solve.py` | `/home/hacker/payload.bin` |
| `msfvenom-no-buffer` | `modulo-17-msfvenom-no-buffer/msfvenom-no-buffer/solve.py` | `/home/hacker/payload.bin` |

Nos modulos 10 a 17, o `solve.py` tambem compila o `vuln.c` correspondente (em
`/tmp`) para ler enderecos/offsets de forma deterministica antes de montar o payload.

Exemplo completo para o primeiro challenge:

```bash
cat > /home/hacker/solve.py
```

Cole o conteudo de `modulo-01-exit42/exit42/solve.py`, finalize com `Ctrl+D`, e execute:

```bash
python3 /home/hacker/solve.py
ls -l /home/hacker/payload.bin
/challenge/check
```

O aluno nao cola os bytes do `.bin` em nenhum lugar. O `solve.py` gera o `.bin` e o `/challenge/check` le o `.bin` automaticamente.

## Publicacao

Crie um dojo privado no pwn.college apontando para este repositorio. O formato segue o repositorio `arch-comp-computer-language`: `dojo.yml` registra os modulos, cada `module.yml` registra um challenge e cada challenge oferece `DESCRIPTION.md` e `/challenge/check`.

Os desafios assumem Linux x86-64 e `pwntools` disponivel no ambiente do aluno.# arch-comp-shellcode
