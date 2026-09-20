# Shellcode com pwntools

Dojo privado para praticar os conceitos de `Aula9_Shellcode_Roteiro.md` no pwn.college.

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

## Fluxo do aluno

1. Entre no desafio no pwn.college.
2. Crie `/home/hacker/solve.py`.
3. Dentro dele, escreva o Assembly pedido como texto dentro de `asm("""...")`.
4. Execute `python3 /home/hacker/solve.py`; o `pwntools` monta o Assembly e grava os bytes crus em `/home/hacker/payload.bin`.
5. Rode `/challenge/check`; o checker compila o harness C, executa o `.bin` e verifica o resultado.

O arquivo `payload.bin` deve conter bytes crus, sem cabecalho ELF. Voce nao precisa criar um arquivo `.asm` separado: o Assembly fica dentro do `solve.py`. No modulo staged, o `solve.py` tambem cria `stage2.bin`. Python aparece somente no lado do aluno, por causa do `pwntools`; o carregador e o harness sao C.

## Publicacao

Crie um dojo privado no pwn.college apontando para este repositorio. O formato segue o repositorio `arch-comp-computer-language`: `dojo.yml` registra os modulos, cada `module.yml` registra um challenge e cada challenge oferece `DESCRIPTION.md` e `/challenge/check`.

Os desafios assumem Linux x86-64 e `pwntools` disponivel no ambiente do aluno.# arch-comp-shellcode
