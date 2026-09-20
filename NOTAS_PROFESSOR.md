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

- exploracao de um buffer overflow real;
- descoberta de offset ate o retorno;
- bypass de ASLR, NX ou PIE;
- reverse shell real ou bind shell real;
- persistencia, criacao de usuario, desativacao de defesa ou alteracao de outro processo.

Esses temas aparecem no roteiro como contexto. No dojo, foram substituidos por harnesses controlados para que a aula ensine shellcode sem fornecer um canal de ataque contra uma maquina real.

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

## 5. Ordem sugerida para a aula

1. Mostrar o modulo 1 e explicar bytes crus, syscall e evidencia de sucesso.
2. Pedir que o aluno localize `solve.py`, rode-o e observe a criacao de `payload.bin`.
3. Mostrar o harness C apenas depois que o fluxo funcionar, relacionando `mmap` ao harness do roteiro.
4. Passar ao modulo 2 e desenhar `argv` e a string na pilha.
5. Usar o modulo 3 para introduzir bad characters e comparar os bytes.
6. Usar o modulo 4 para mostrar que shellcode pode ter efeito unico.
7. Usar o modulo 5 para explicar decoder, dados em memoria e assinaturas.
8. Usar o modulo 6 para explicar limite de tamanho e staged payload.
9. Fechar com o modulo 7 e separar transporte local de reverse/bind shell.

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

## 7. Limites e responsabilidade

Todos os testes devem ocorrer no container do pwn.college, em laboratorio autorizado. Os exemplos de rede, persistencia, criacao de usuario e desativacao de defesa do roteiro sao conceitos para discussao defensiva e nao fazem parte destes desafios. O aluno deve entender que executar shellcode fora de um ambiente autorizado pode causar impacto real e nao deve ser feito.
