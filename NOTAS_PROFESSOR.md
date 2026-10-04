# Notas do professor - Dojo de Shellcode com pwntools

Este arquivo explica o que cada desafio ensina, qual e o papel do atacante e do defensor, qual metodo tecnico esta sendo praticado e como conduzir a atividade. Ele complementa o `Aula9_Shellcode_Roteiro.md` e nao deve ser apresentado como a resposta do aluno.

## 1. Visao geral da aula

### Objetivo

Fazer o aluno entender a cadeia completa:

```text
Assembly -> pwntools -> payload.bin -> harness C -> execucao do shellcode -> evidencia -> flag
```

O aluno nao escreve o harness C. O harness ja esta no challenge e serve como bancada controlada para executar os bytes. O aluno escreve somente o `solve.py`, que usa `pwntools` para transformar Assembly em bytes crus.

### O que o aluno pratica

- relacao entre Assembly, opcode e shellcode;
- registradores usados nas syscalls Linux x86-64;
- montagem de strings e ponteiros na pilha;
- diferenca entre bytes crus e um executavel ELF;
- position-independent code;
- restricoes de bad characters;
- payload stageless, encoded e staged;
- leitura de evidencias e validacao automatica.

### O que nao e praticado neste dojo

- reverse shell real ou bind shell real (nenhum socket, listener ou conexao externa);
- bypass de ASLR, NX ou PIE em um alvo com as defesas ligadas;
- persistencia, criacao de usuario, desativacao de defesa ou alteracao de outro processo;
- qualquer ataque fora do container do laboratorio.

Esses temas aparecem no roteiro como contexto. No dojo, foram substituidos por harnesses controlados para que a aula ensine shellcode sem fornecer um canal de ataque contra uma maquina real.

### Extensao: Metasploit, corrupcao de memoria e buffer overflow (modulos 8-17)

A partir do modulo 8 o dojo foi estendido para cobrir o que antes era apenas
contexto. A exploracao de buffer overflow, a descoberta de offset e a injecao de
shellcode na pilha **passam a ser praticadas**, sempre de forma local e controlada:

- **Modulos 8-9 (Metasploit):** apresentam o framework e o `msfvenom` como gerador
  de payloads (o analogo do `pwntools`). A instalacao e feita na hora, passo a passo.
  Cada desafio aceita um fallback em `pwntools`, porque o checker valida o
  comportamento do payload, nao a ferramenta usada para gera-lo.
- **Modulos 10-13 (corrupcao de memoria - primitivas):** isolam cada mecanismo de
  corrupcao em um programa C vulneravel: variavel adjacente, ponteiro de funcao,
  endereco de retorno e NOP sled. Aqui o aluno aprende *o que* se corrompe, um
  mecanismo de cada vez, sem ainda montar a cadeia completa.
- **Modulos 14-17 (buffer overflow - cadeia completa):** juntam as primitivas na
  exploracao completa: descobrir o offset com `cyclic`, controlar o RIP, injetar
  shellcode com sled e, por fim, entregar um payload do `msfvenom` pelo overflow.

Decisoes didaticas importantes de seguranca nesses modulos:

- Os `vuln.c` sao compilados com `-fno-stack-protector -no-pie` e, nos modulos 15-17,
  `-z execstack`. As defesas sao desligadas **de proposito**, para que o mecanismo
  fique visivel; o professor deve enfatizar que cada flag desligada corresponde a uma
  defesa real (canary, PIE/ASLR, NX).
- Os programas vulneraveis **dropam privilegios** (para uid 1000) antes de executar o
  fluxo controlado pelo aluno. Assim, mesmo o shell do modulo 16 roda sem privilegio.
- Nos modulos 15-17, o alvo desliga ASLR e fixa o ambiente re-executando a si mesmo
  (`personality(ADDR_NO_RANDOMIZE)` + ambiente minimo). Isso torna o endereco do
  buffer deterministico: e um artificio de laboratorio para o exercicio ser
  reprodutivel, nao uma condicao de um alvo real.

## 2. Papeis: atacante e defensor

### Papel do atacante

Neste ambiente didatico, o aluno assume o papel do autor do payload. Ele precisa:

1. escrever a logica em Assembly;
2. montar a logica com `pwntools`;
3. respeitar o formato e os limites do desafio;
4. entregar os bytes em `payload.bin`;
5. demonstrar que o comportamento esperado ocorreu.

O `solve.py` e uma ferramenta de construcao e entrega. Ele nao e o shellcode em si. O shellcode e a sequencia de bytes gravada no `.bin`.

### Papel do defensor

O defensor e representado pelo harness e pelo checker. O ambiente:

- executa o payload em um container isolado;
- reduz privilegios para `hacker` quando o harness inicia como root;
- limita tamanho dos arquivos;
- rejeita bytes proibidos quando o desafio pede isso;
- verifica uma evidencia especifica, em vez de aceitar qualquer comportamento;
- nao permite que o exercicio dependa de uma maquina externa.

Em um sistema real, a defesa correspondente seria usar NX/DEP, ASLR, PIE, stack canaries, validacao de tamanho, filtros de entrada, sandboxing, seccomp, EDR e logs. O checker demonstra a ideia de validacao; ele nao e uma defesa completa de producao.

### Por que os dois papeis importam

O atacante pensa: "qual sequencia minima de bytes produz o efeito pedido sob estas restricoes?"

O defensor pensa: "qual comportamento aceito prova o objetivo sem permitir efeitos fora do escopo?"

Essa dualidade evita que shellcode seja ensinado apenas como uma lista de instrucoes. Cada tecnica existe porque ha uma restricao, uma oportunidade ou uma defesa para enfrentar.

## 3. Fluxo tecnico comum a todos os desafios

### Antes da execucao

O aluno cria:

```text
/home/hacker/solve.py
```

Dentro dele, o Assembly aparece como texto:

```python
payload = asm("""
    ... instrucoes Assembly ...
""")
```

`pwntools` chama o montador e produz bytes de maquina. O Python grava esses bytes em:

```text
/home/hacker/payload.bin
```

O aluno nao cria `.asm`, ELF ou harness C.

### Durante o checker

O `/challenge/check` e um script do ambiente. Ele compila o harness C e executa algo equivalente a:

```text
abrir payload.bin
mmap memoria com permissao de execucao
carregar os bytes
converter o endereco em ponteiro de funcao
executar o shellcode
verificar a evidencia
```

Essa etapa simula a bancada de testes do roteiro. Em um exploit real, a origem dos bytes poderia ser um buffer vulneravel; aqui a origem e um arquivo fornecido diretamente pelo aluno.

### Mensagem que o professor deve reforcar

`payload.bin` nao e um programa completo. Ele nao tem cabecalho ELF, entry point ou bibliotecas. Ele contem somente os bytes que serao executados a partir do endereco escolhido pelo harness.

### Demonstração didática: quais bytes foram gerados

Depois de executar o `solve.py`, o professor pode mostrar o tamanho e o conteúdo bruto do payload:

```bash
wc -c /home/hacker/payload.bin
od -An -tx1 -v /home/hacker/payload.bin
```

No modulo 1, a saída esperada é um arquivo de **12 bytes**:

```text
b8 3c 00 00 00 bf 2a 00 00 00 0f 05
```

O comando `wc -c` conta bytes. O comando `od -An -tx1 -v` mostra cada byte em hexadecimal, sem tentar interpretar o arquivo como texto ou ELF. A relação com o Assembly pode ser mostrada assim:

| Bytes | Instrução | Significado |
|---|---|---|
| `b8 3c 00 00 00` | `mov eax, 60` | seleciona `exit` |
| `bf 2a 00 00 00` | `mov edi, 42` | define o status `42` |
| `0f 05` | `syscall` | entra no kernel |

Os zeros nesse payload não são instruções separadas. Eles são parte da codificação little-endian e do preenchimento das instruções `mov` com registradores de 32 bits. Por isso o módulo 3 usa outra forma de montar os registradores.

### Vulnerabilidade, mecanismo e consequência

Marcador para busca: `Vulnerabilidade, mecanismo e consequencia`.

O professor deve separar três perguntas que costumam ser misturadas:

1. **Qual vulnerabilidade ou mecanismo entregou os bytes?** No harness, nenhuma vulnerabilidade real: o programa abre o arquivo e decide explicitamente executar seu conteúdo. Em um exploit real, a entrega poderia ocorrer por um buffer overflow que sobrescreve o endereço de retorno, por exemplo `read(0, buffer, 100)` em um buffer de 16 bytes.
2. **O que foi gerado tecnicamente?** Um desvio de fluxo para bytes controlados pelo aluno, seguido da execução de instruções escolhidas por ele.
3. **Qual foi a consequência?** Depende do shellcode: terminar o processo, substituir o processo por um shell, escrever uma mensagem ou carregar um segundo estágio.

No laboratório, o harness substitui a vulnerabilidade apenas para isolar o estudo do shellcode. Assim, o aluno aprende a produzir e executar os bytes antes de combinar isso com uma exploração de memória real.

## 4. Fichas dos desafios

## Modulo 1 - `exit42`

### Metodo

Shellcode stageless de prova de conceito usando a syscall `exit`.

### O que o aluno faz

Escreve `mov eax, 60`, coloca `42` em `edi` e executa `syscall`. O `solve.py` grava o resultado em `payload.bin`.

### O que o atacante esta tentando demonstrar

Que consegue controlar o fluxo de execucao e escolher o codigo de termino do processo. O objetivo nao e abrir shell; e provar o menor efeito observavel possivel.

### Bytes, entrega e consequência

Marcador para busca: `Bytes, entrega e consequencia`.

- **Bytes:** normalmente 12 bytes; mostrar com `wc -c /home/hacker/payload.bin` e `od -An -tx1 -v /home/hacker/payload.bin`.
- **Vulnerabilidade explorada:** nenhuma no harness. A entrega e intencional e controlada por `mmap` + leitura do arquivo + ponteiro de funcao. Em um exploit real, um buffer overflow poderia sobrescrever o retorno para apontar ao buffer.
- **Problema gerado:** desvio de fluxo controlado e encerramento do processo com status `42`. Nao houve corrupção acidental de memória neste desafio.

### O que o defensor/checker verifica

O processo termina com status `42`. Um payload que apenas imprime algo ou retorna normalmente nao atende ao objetivo.

### Por que comecar por aqui

E o menor ciclo completo: Assembly, syscall, bytes, carregamento e evidencia. Ele separa o problema de escrever shellcode do problema posterior de montar argumentos complexos.

### Quando esse conceito aparece

Em testes de controle de fluxo, validacao de que uma area foi executada e provas de conceito minimas. Para defesa, um processo terminando de modo anormal e uma evidencia que pode aparecer em logs, mas nao e por si so uma deteccao confiavel.

## Modulo 2 - `spawn-shell`

### Metodo

Shellcode stageless de spawn shell usando `execve("/bin/sh", argv, NULL)`.

### O que o aluno faz

Monta a string `/bin//sh` na pilha, cria o vetor `argv`, configura `rdi`, `rsi` e `rdx`, coloca 59 em `rax` e executa `syscall`.

### O que o atacante esta tentando demonstrar

Que consegue substituir a imagem do processo por um shell. O shell herda stdin e stdout do harness, por isso o checker consegue enviar um comando controlado e observar a resposta.

### Bytes, entrega e consequência

- **Bytes:** o tamanho depende da montagem; mostrar com `wc -c` e `od -An -tx1 -v`.
- **Vulnerabilidade explorada:** nenhuma no harness. O mecanismo didatico e a execução deliberada do arquivo. Em um exploit real, o shellcode poderia ser escrito em um buffer overflow e o retorno apontado para esse buffer.
- **Problema gerado:** `execve` substitui a imagem do processo por `/bin/sh`; o programa original deixa de executar, mas o processo mantém os privilegios e descritores que possuía.

### O que o defensor/checker verifica

O checker envia `printf SHELL_OK; exit` e procura `SHELL_OK`. Ele nao aceita apenas a existencia de um arquivo ou um retorno sem evidencia de que `execve` funcionou.

### Por que vem depois de `exit42`

O aluno ja sabe executar uma syscall. Agora precisa construir dados em memoria e passar ponteiros para uma syscall com tres argumentos.

### Quando esse conceito aparece

Em exploits autorizados onde o objetivo e obter um interpretador local. Em defesa, spawn de shell por processos inesperados, especialmente com descritores herdados, e um sinal importante para investigacao.

## Modulo 3 - `null-free`

### Metodo

Shellcode de spawn shell com restricao de bad character: nenhum byte `0x00`.

### O que o aluno faz

Repete o objetivo do modulo 2, mas substitui formas de montagem que geram zeros literais. A forma didatica principal e zerar com `xor` e carregar o numero pequeno com `mov al, 59`.

### O que o atacante esta tentando demonstrar

Que consegue adaptar o mesmo comportamento ao canal de entrada. Em uma vulnerabilidade baseada em strings, `0x00` pode encerrar a copia e impedir que o restante do payload chegue ao destino.

### Bytes, entrega e consequência

- **Bytes:** mostrar com `wc -c` e `od -An -tx1 -v`; confirmar a restrição com `od -An -tx1 -v payload.bin | grep -w 00`.
- **Vulnerabilidade explorada:** nenhuma no harness; a vulnerabilidade discutida e um canal real baseado em strings, como uso incorreto de `strcpy`, que trunca a entrada no primeiro `0x00`.
- **Problema gerado:** no canal vulnerável, o shellcode seria copiado de forma incompleta e poderia perder a syscall final. A versão null-free evita esse truncamento e ainda produz o shell.

### O que o defensor/checker verifica

Primeiro inspeciona o arquivo e rejeita qualquer `0x00`. Depois executa o shell e procura `NULL_FREE_OK`.

### Por que existe

O comportamento final pode ser identico ao modulo 2, mas a representacao dos bytes muda. Essa e a ponte entre Assembly legivel e as restricoes reais do transporte.

### Quando esse conceito aparece

Em entradas copiadas por `strcpy`, argumentos ou protocolos que tratam zero como terminador. Para defesa, validar e normalizar entradas e impedir memoria gravavel-executavel reduz a utilidade desse payload.

## Modulo 4 - `comando-unico`

### Metodo

Payload stageless de efeito unico usando a syscall `write`, sem shell interativo.

### O que o aluno faz

Coloca a mensagem no proprio payload, usa RIP-relative addressing para encontrar a mensagem e chama `write(1, mensagem, tamanho)`. Depois encerra.

### O que o atacante esta tentando demonstrar

Que um payload nao precisa abrir shell. Ele pode executar uma acao especifica e terminar, reduzindo tamanho e ruido operacional.

### Bytes, entrega e consequência

- **Bytes:** mostrar com `wc -c` e `od -An -tx1 -v`; a mensagem aparece como dados dentro do próprio payload.
- **Vulnerabilidade explorada:** nenhuma no harness. O mecanismo real estudado é a possibilidade de desviar a execução para bytes injetados, sem depender de uma função útil já existente no binário.
- **Problema gerado:** uma ação única e observável via `write`, seguida do encerramento; não há shell interativo nem processo persistente.

### O que o defensor/checker verifica

A saida precisa ser exatamente `COMANDO_UNICO_OK` com quebra de linha. Saida extra indica que o payload nao respeitou o contrato.

### Por que existe

Corrige a ideia de que todo shellcode e um spawn shell. Tambem introduz dados embutidos e codigo position-independent.

### Quando esse conceito aparece

Em stagers pequenos, probes, validacoes de controle de fluxo e payloads que precisam produzir uma acao curta. Para defesa, saidas inesperadas e syscalls fora do perfil normal podem alimentar deteccao comportamental.

## Modulo 5 - `encoded`

### Metodo

Shellcode codificado: decoder XOR position-independent seguido de estagio codificado.

### O que o aluno faz

Monta um estagio que escreve `ENCODED_OK`, aplica XOR aos bytes e cria um decoder que percorre os bytes em memoria, desfaz a codificacao e salta para o estagio recuperado.

### O que o atacante esta tentando demonstrar

Que pode alterar a representacao do payload para evitar uma assinatura simples ou uma restricao de bytes. A codificacao nao e criptografia forte nem torna o payload seguro.

### Bytes, entrega e consequência

- **Bytes:** mostrar com `wc -c` e `od -An -tx1 -v`; procurar a string em claro com `grep -aF ENCODED_OK payload.bin`, que deve não encontrar resultado.
- **Vulnerabilidade explorada:** nenhuma no harness. Em um cenário real, o mecanismo de entrega poderia ser uma corrupção de memória que aceita os bytes codificados, mas sofre com filtros ou assinaturas.
- **Problema gerado:** o decoder modifica bytes em memória e transfere o controle para o estágio recuperado. A consequência é a execução do payload real depois de sua representação armazenada ter sido transformada.

### O que o defensor/checker verifica

A string `ENCODED_OK` nao pode estar em claro no arquivo. Depois o payload precisa decodificar e produzir a saida esperada.

### Por que existe

Ensina a diferenca entre payload armazenado e payload executado. O decoder e a primeira parte executada; o estagio real so existe depois da transformacao em runtime.

### Quando esse conceito aparece

Em loaders, packers e malware, alem de exercicios de bad characters. Para defesa, procurar apenas strings conhecidas e insuficiente; deve-se observar decodificacao em memoria, paginas gravaveis-executaveis e comportamento de syscalls.

## Modulo 6 - `staged`

### Metodo

Payload staged: stage 1 pequeno recebe o stage 2 pela entrada e o executa.

### O que o aluno faz

Gera `payload.bin` com no maximo 64 bytes e `stage2.bin` com o payload que imprime `STAGED_OK`. O stage 1 usa `mmap`, `read` e um salto para a memoria recebida.

### O que o atacante esta tentando demonstrar

Que consegue superar uma limitacao de tamanho. O primeiro estagio cabe no espaco pequeno; o restante chega depois por um canal ja aberto.

### Bytes, entrega e consequência

- **Bytes:** mostrar separadamente com `wc -c /home/hacker/payload.bin`, `od -An -tx1 -v /home/hacker/payload.bin`, `wc -c /home/hacker/stage2.bin` e `od -An -tx1 -v /home/hacker/stage2.bin`.
- **Vulnerabilidade explorada:** nenhuma no harness. A situação real simulada é uma vulnerabilidade que permite gravar somente poucos bytes no primeiro momento e mantém um canal para receber mais dados.
- **Problema gerado:** o stage 1 lê o stage 2 para memória executável e pula para ele. O payload completo não precisa caber no espaço inicial.

### O que o defensor/checker verifica

Limita os tamanhos, envia o stage 2 pela entrada padrao e confirma que o stage 1 realmente o le e transfere a execucao.

### Por que existe

Mostra por que shellcode real e dividido em etapas. O stage 1 prioriza tamanho e capacidade de carregar dados; o stage 2 prioriza funcionalidade.

### Quando esse conceito aparece

Em vulnerabilidades com poucos bytes disponiveis, bootstraps e loaders. Para defesa, a combinacao de leitura de dados, mudanca de permissao de memoria e salto para dados e um padrao de alto interesse.

## Modulo 7 - `canal-local`

### Metodo

Spawn shell usando stdin/stdout ja estabelecidos, sem criar socket.

### O que o aluno faz

Executa `/bin/sh` preservando os descritores 0 e 1. O aluno compara esse canal herdado com reverse shell e bind shell, mas nao implementa conexao de rede.

### O que o atacante esta tentando demonstrar

Que o transporte e separado do payload. O shellcode pode receber comandos por um canal que ja existe; ele nao precisa criar um novo canal.

### Bytes, entrega e consequência

- **Bytes:** mostrar com `wc -c /home/hacker/payload.bin` e `od -An -tx1 -v /home/hacker/payload.bin`.
- **Vulnerabilidade explorada:** nenhuma no harness. A situação real simulada é um processo comprometido que já possui stdin/stdout conectados a um terminal, pipe ou serviço.
- **Problema gerado:** o processo passa a executar um shell através dos descritores herdados. Não há `socket`, `connect`, `bind` ou `listen` neste challenge.

### O que o defensor/checker verifica

Envia `printf CANAL_LOCAL_OK; exit` pela entrada e procura a resposta na saida.

### Por que existe

Evita que o aluno confunda objetivo do shellcode com mecanismo de comunicacao. Reverse shell e bind shell pertencem a outro exercicio, com rede controlada e autorizada.

### Quando esse conceito aparece

Em processos comprometidos que ja possuem stdin/stdout redirecionados por um servico, pipe ou terminal. Para defesa, processos inesperadamente transformados em shells e uso anormal de descritores herdados merecem investigacao.

## Modulo 8 - `msf-setup`

### Metodo

Apresentacao do Metasploit Framework e geracao do primeiro payload com `msfvenom`,
executado no mesmo harness controlado dos modulos de shellcode.

### O que o aluno faz

Instala o framework na hora pelo script oficial (omnibus), confirma com
`msfvenom --version` e gera `linux/x64/exec CMD='/bin/echo MSF_SETUP_OK' -f raw`. O
harness carrega e executa os bytes. Ha um `solve.py` de fallback em `pwntools` para
quando nao houver rede/instalacao.

> **Pegadinha de ambiente (pwn.college):** o workspace padrao e nao-privilegiado -
> `sudo`/`apt` retornam `workspace is not privileged`. A instalacao do Metasploit
> exige reiniciar o desafio no **workspace privilegiado** (Practice), que da root mas
> **nao pontua** a flag. Por isso o fallback em `pwntools` existe: ele roda no
> workspace padrao e pontua. Em sala, demonstrar o `msfvenom` no modo privilegiado e
> pontuar com o fallback no modo normal.

### O que o atacante esta tentando demonstrar

Que uma ferramenta pode automatizar o que ele fez a mao nos modulos 1 a 7. O mapa
mental e: payload = shellcode; `msfvenom` = `pwntools`; encoder = modulo 5; stager =
modulo 6; `msfconsole` = orquestrador (so conceito aqui).

### O que o defensor/checker verifica

Que a saida contem `MSF_SETUP_OK`. Nao importa se veio do `msfvenom` ou do fallback.

### Por que comeca o bloco

Serve de ponte: o aluno ja sabe o que e shellcode; agora ve a ferramenta padrao da
industria gerando o mesmo tipo de bytes.

## Modulo 9 - `msf-encoders`

### Metodo

Uso de encoder, formato de saida e restricao de bad chars no `msfvenom`, ligando ao
modulo 3 (`null-free`) e ao modulo 5 (`encoded`).

### O que o aluno faz

Gera um payload sem nenhum `0x00` (`-b '\x00'`, opcionalmente `-e x64/...`) que
produz `MSF_ENC_OK`. O fallback em `pwntools` monta a string em runtime, sem zeros.

### O que o atacante esta tentando demonstrar

Que pode adaptar a representacao dos bytes a restricoes do canal (bad chars) sem
mudar o efeito. Deve-se reforcar: encoder serve para restricao de bytes, nao para
"ficar indetectavel".

### O que o defensor/checker verifica

Rejeita qualquer `0x00` no arquivo e depois procura `MSF_ENC_OK`.

## Modulo 10 - `var-adjacente`

### Metodo

Primitiva de corrupcao: estouro de buffer sobre uma variavel adjacente na mesma
struct, sem controle de fluxo.

### O que o aluno faz

Envia 32 bytes de preenchimento + 8 bytes com `0x1337` para sobrescrever o campo
`admin` que fica logo apos `name`. O `vuln.c` usa `memcpy` com o tamanho do arquivo.

### O que o atacante esta tentando demonstrar

Que "confiar no tamanho da entrada" ja e exploravel mesmo sem desviar a execucao: a
variavel ao lado pode ser uma flag de autenticacao.

### O que o defensor/checker verifica

Compila com `-fno-stack-protector -no-pie`, roda e procura `VAR_ADJ_OK` (so impresso
quando `admin == 0x1337`). Defesa real: usar `sizeof` do destino, funcoes com limite,
canaries.

## Modulo 11 - `ponteiro-funcao`

### Metodo

Primitiva de corrupcao: sobrescrever um ponteiro de funcao guardado em dados e
desviar a chamada que o programa faz.

### O que o aluno faz

Enche `buf[32]` e sobrescreve o ponteiro `handler` (que apontava para `safe`) com o
endereco de `win()`. O endereco e lido do binario (deterministico por `-no-pie`); o
`solve.py` compila o fonte e le o simbolo com `ELF`.

### O que o atacante esta tentando demonstrar

Primeiro passo rumo ao controle de fluxo: ja nao e um numero, e um alvo de salto que
o proprio programa executa (analogia com vtables, callbacks e GOT).

### O que o defensor/checker verifica

Procura `FUNC_PTR_OK` (impresso por `win()`). Defesa real: CFI, RELRO, layout.

## Modulo 12 - `endereco-retorno`

### Metodo

Primitiva de corrupcao: sobrescrever o endereco de retorno salvo na pilha (ret2win),
com o offset fornecido.

### O que o aluno faz

Envia 72 bytes (64 do buffer + 8 do rbp salvo) + o endereco de `win()`. Quando
`vuln()` executa `ret`, salta para `win()`. `win()` usa `write` + `_exit` para a
evidencia sair antes de qualquer crash.

### O que o atacante esta tentando demonstrar

O coracao do stack smashing: quem controla o endereco de retorno controla o fluxo
apos a funcao terminar. Aqui ainda saltamos para codigo existente, nao injetado.

### O que o defensor/checker verifica

Procura `RET2WIN_OK`. Defesa real: stack canary, ASLR, NX. O modulo nomeia o offset;
descobri-lo e assunto do modulo 14.

## Modulo 13 - `nop-sled`

### Metodo

Primitiva de confiabilidade: NOP sled para tolerar imprecisao no endereco de pouso.

### O que o aluno faz

Monta `[256 NOPs][shellcode que imprime NOP_SLED_OK]`. O harness pula para um ponto
aleatorio (jitter de ate 200 bytes); como o sled cobre o jitter, o pouso sempre
alcanca o shellcode. O checker roda 5 vezes.

### O que o atacante esta tentando demonstrar

Que precisao pode ser trocada por margem. Prepara os modulos de buffer overflow.

### O que o defensor/checker verifica

`NOP_SLED_OK` em todas as 5 execucoes. Para a defesa, longas sequencias de `0x90` sao
um indicador classico de IDS/EDR.

## Modulo 14 - `offset`

### Metodo

Buffer overflow completo, etapa 1: descobrir o offset ate o endereco de retorno com
padrao ciclico (`cyclic`).

### O que o aluno faz

Gera `cyclic(400)`, roda sob o `gdb`, observa o valor que o `ret` tentou usar e
traduz com `cyclic_find`. Neste binario o offset e 120; o objetivo e o metodo.

### O que o atacante esta tentando demonstrar

Que o offset nao precisa ser dado: ha uma tecnica sistematica para encontra-lo.

### O que o defensor/checker verifica

Procura `OFFSET_OK`. Offset errado nao salta para `win()`.

## Modulo 15 - `controle-rip`

### Metodo

Buffer overflow completo, etapa 2: controlar o RIP e saltar para o proprio buffer
(shellcode na pilha).

### O que o aluno faz

Le o endereco do buffer no `[leak]` (o `solve.py` automatiza), monta
`[shellcode][NOPs ate 136][endereco de retorno = buf]` e o `ret` entra no buffer. O
shellcode imprime `RIP_OK`.

### O que o atacante esta tentando demonstrar

A juncao das duas metades do dojo: corrupcao (endereco de retorno) + shellcode
(bytes do aluno). Enfatizar que so funciona porque NX, ASLR e canary estao desligados.

### O que o defensor/checker verifica

Compila com `-z execstack` e procura `RIP_OK`.

## Modulo 16 - `shellcode-injetado`

### Metodo

Buffer overflow completo, etapa 3: injetar `execve("/bin/sh")` com NOP sled e abrir
shell pelo canal herdado.

### O que o aluno faz

Monta `[sled][execve /bin/sh][NOPs][ret = buf]`. O shell herda o stdin; o checker
envia `echo BOF_SHELL_OK`. E o mesmo shellcode do modulo 2, agora entregue por
overflow.

### O que o atacante esta tentando demonstrar

A cadeia completa de injecao de codigo: corromper o retorno, pousar no sled, executar
shellcode arbitrario com os privilegios e descritores do processo.

### O que o defensor/checker verifica

Procura `BOF_SHELL_OK` na saida do shell. Defesa que mata este caminho: NX.

## Modulo 17 - `msfvenom-no-buffer`

### Metodo

Buffer overflow completo, etapa 4: entregar um shellcode gerado pelo `msfvenom` pelo
overflow. Fecha o ciclo Metasploit + buffer overflow.

### O que o aluno faz

Gera o shellcode com `msfvenom -p linux/x64/exec CMD='/bin/echo MSF_BUF_OK'` (ou
fallback em `pwntools`), embrulha em `[sled][shellcode][NOPs][ret = buf]` e entrega
pelo buffer.

### O que o atacante esta tentando demonstrar

Que as pecas estudadas separadamente se combinam: ferramenta gera, vulnerabilidade
entrega, pilha executavel roda.

### O que o defensor/checker verifica

Procura `MSF_BUF_OK`. Aceita `msfvenom` ou o fallback.

## 5. Ordem sugerida para a aula

1. Mostrar o modulo 1 e explicar bytes crus, syscall e evidencia de sucesso.
2. Pedir que o aluno localize `solve.py`, rode-o e observe a criacao de `payload.bin`.
3. Mostrar o harness C apenas depois que o fluxo funcionar, relacionando `mmap` ao harness do roteiro.
4. Passar ao modulo 2 e desenhar `argv` e a string na pilha.
5. Usar o modulo 3 para introduzir bad characters e comparar os bytes.
6. Usar o modulo 4 para mostrar que shellcode pode ter efeito unico.
7. Usar o modulo 5 para explicar decoder, dados em memoria e assinaturas.
8. Usar o modulo 6 para explicar limite de tamanho e staged payload.
9. Fechar o bloco de shellcode com o modulo 7 e separar transporte local de reverse/bind shell.
10. Introduzir o Metasploit (modulo 8): instalar na hora e mostrar que `msfvenom` gera o mesmo tipo de bytes do `pwntools`.
11. Usar o modulo 9 para ligar encoders/bad chars (`-b`, `-e`) ao que ja foi visto nos modulos 3 e 5.
12. Abrir o bloco de corrupcao (modulo 10) mostrando o `vuln.c`: desenhar `name` e `admin` lado a lado na pilha.
13. Modulo 11: transformar a variavel vizinha em um ponteiro de funcao e mostrar `nm`/`objdump` achando `win`.
14. Modulo 12: desenhar o frame (buffer, rbp salvo, endereco de retorno) e fazer o primeiro ret2win.
15. Modulo 13: introduzir o NOP sled e a ideia de tolerancia a imprecisao.
16. Modulo 14: ensinar `cyclic`/`cyclic_find` e a descoberta de offset sob o `gdb`.
17. Modulo 15: controlar o RIP e saltar para o buffer; discutir por que NX/ASLR/canary estao desligados.
18. Modulo 16: injetar o `execve("/bin/sh")` com sled e abrir shell pelo stdin herdado.
19. Fechar com o modulo 17: entregar um payload do `msfvenom` pelo overflow e amarrar os dois blocos.

Em cada modulo de 10 a 17, mostrar o `vuln.c` **antes** da solucao e pedir que o
aluno aponte a linha vulneravel e a defesa que a evitaria.

## 6. Perguntas de verificacao para o professor

- Qual parte do `solve.py` e Python e qual parte e Assembly?
- Qual arquivo o `solve.py` gera?
- Quem le `payload.bin`?
- O aluno precisa escrever o harness C?
- Por que `payload.bin` nao e um ELF?
- Qual syscall e usada e quais registradores carregam seus argumentos?
- Qual restricao muda entre os modulos 2 e 3?
- Por que o modulo 5 precisa de um decoder?
- Por que o stage 1 existe no modulo 6?
- Qual e a diferenca entre canal de comunicacao e objetivo do payload?
- Que defesa impediria o harness de executar dados como codigo?
- Qual e o equivalente no Metasploit do `pwntools`, do decoder (modulo 5) e do stager (modulo 6)?
- Por que o checker aceita tanto `msfvenom` quanto o fallback em `pwntools`?
- Qual a diferenca entre sobrescrever uma variavel adjacente, um ponteiro de funcao e o endereco de retorno?
- Como o `cyclic` transforma a descoberta do offset em algo sistematico?
- Por que o NOP sled aumenta a confiabilidade do exploit?
- Quais flags de compilacao desligam quais defesas nos `vuln.c`, e o que cada defesa faria se estivesse ligada?
- Por que o alvo dos modulos 15-17 desliga ASLR e fixa o ambiente, e por que isso e um artificio de laboratorio?

## 7. Limites e responsabilidade

Todos os testes devem ocorrer no container do pwn.college, em laboratorio autorizado. Os exemplos de rede, persistencia, criacao de usuario e desativacao de defesa do roteiro sao conceitos para discussao defensiva e nao fazem parte destes desafios.

Os modulos 10 a 17 introduzem corrupcao de memoria e buffer overflow com as defesas (canary, PIE/ASLR, NX) desligadas de proposito, para tornar o mecanismo visivel. Isso **nao** representa um sistema real: um alvo de producao tem essas protecoes ligadas, e os `vuln.c` existem apenas como bancada didatica dentro do container. O Metasploit e instalado e usado somente neste ambiente autorizado. O aluno deve entender que gerar, injetar ou executar shellcode, ou explorar qualquer vulnerabilidade, fora de um ambiente autorizado pode causar impacto real e nao deve ser feito.
