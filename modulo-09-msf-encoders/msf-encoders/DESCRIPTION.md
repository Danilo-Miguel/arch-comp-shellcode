# msfvenom: encoders, formatos e bad chars

Este modulo liga o `msfvenom` ao que voce viu no modulo 3 (`null-free`) e no modulo
5 (`encoded`). O objetivo e gerar um payload que:

1. **nao contenha nenhum byte `0x00`** (bad char), e
2. produza a string exata `MSF_ENC_OK`.

## Conceitos

- **Formato (`-f`)**: como o payload sai. `raw` = bytes crus (o que o harness le).
  Outros formatos (`-f c`, `-f python`, `-f hex`) sao so representacoes para colar
  em codigo; nao mudam os bytes executados.
- **Bad chars (`-b`)**: bytes que nao podem aparecer no payload porque o canal de
  entrega os corromperia. `0x00` termina strings em C (modulo 3); `0x0a` pode ser
  tratado como fim de linha. `-b '\x00'` diz ao msfvenom para evitar esses bytes.
- **Encoder (`-e`)**: transforma os bytes do payload e adiciona um decoder que
  desfaz a transformacao em runtime (exatamente a ideia do modulo 5). Encoders
  servem para evitar bad chars, nao para "ficar indetectavel".

## Caminho com msfvenom

> Requer o Metasploit instalado, ou seja, o **workspace privilegiado** (ver modulo 8:
> no workspace padrao, `sudo`/`apt` nao funcionam). Sem ele, pule para o fallback em
> `pwntools` - o checker valida o comportamento, nao a ferramenta.

```bash
msfvenom -p linux/x64/exec CMD='/bin/echo MSF_ENC_OK' \
         -b '\x00' -f raw -o /home/hacker/payload.bin
```

O `-b '\x00'` faz o msfvenom escolher uma codificacao que nao gere bytes nulos
(pode usar um encoder automaticamente). Confirme:

```bash
od -An -tx1 -v /home/hacker/payload.bin | grep -w 00    # nao deve achar nada
```

Se quiser forcar um encoder explicito e ver o decoder, tente:

```bash
msfvenom -p linux/x64/exec CMD='/bin/echo MSF_ENC_OK' \
         -e x64/xor_dynamic -b '\x00' -f raw -o /home/hacker/payload.bin
```

## Caminho de fallback (pwntools)

Se o Metasploit nao estiver disponivel, o `solve.py` gera um payload equivalente
sem `0x00`, usando as mesmas tecnicas do modulo 3 (zerar com `xor`, carregar valores
pequenos em registradores de 8 bits) e com a mensagem construida em runtime.

## Fluxo completo

```bash
# msfvenom OU python3 /home/hacker/solve.py
od -An -tx1 -v /home/hacker/payload.bin
/challenge/check
```

O checker primeiro rejeita qualquer `0x00` no arquivo, depois executa o payload e
procura `MSF_ENC_OK`.
