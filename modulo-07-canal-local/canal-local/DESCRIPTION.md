# Canal local controlado

Este desafio representa o objetivo de um shell conectado sem criar conexoes de rede: stdin e stdout ja formam um canal local controlado pelo checker.

Gere shellcode em `payload.bin` que execute `/bin/sh` preservando esses descritores. O checker envia `printf CANAL_LOCAL_OK` e espera a resposta. Compare essa entrega com a de um reverse ou bind shell: aqui o transporte ja esta estabelecido, portanto nao ha `socket`, `connect`, `bind` ou `listen`.