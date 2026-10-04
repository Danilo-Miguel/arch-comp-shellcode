#include <fcntl.h>
#include <grp.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/*
 * Primitiva de corrupcao: endereco de retorno.
 *
 * Quando vuln() e chamada, a pilha guarda: o buffer, o rbp salvo e, logo acima,
 * o ENDERECO DE RETORNO - para onde a CPU volta quando vuln() termina (ret).
 * Estourando buf, voce sobrescreve esse endereco e redireciona a execucao ao
 * terminar a funcao. Aqui o alvo e win().
 *
 * Layout do frame de vuln() (gcc -O0, sem canary):
 *   [ buf: 64 bytes ][ rbp salvo: 8 ][ endereco de retorno: 8 ]
 *   offset do inicio de buf ate o endereco de retorno = 64 + 8 = 72
 */

static void drop_privileges(void) {
    if (geteuid() == 0) {
        if (setgroups(0, NULL) != 0 || setgid(1000) != 0 || setuid(1000) != 0) {
            perror("drop privileges");
            exit(1);
        }
    }
}

void win(void) {
    /* write + _exit: saida imediata, sem buffer e sem crash depois do ret. */
    const char msg[] = "RET2WIN_OK\n";
    write(1, msg, sizeof(msg) - 1);
    _exit(0);
}

void vuln(int fd) {
    char buf[64];
    /* LEITURA VULNERAVEL: le ate 256 bytes em um buffer de 64. */
    read(fd, buf, 256);
}

int main(int argc, char **argv) {
    drop_privileges();
    if (argc != 2) {
        fprintf(stderr, "usage: vuln PAYLOAD\n");
        return 1;
    }
    fprintf(stderr, "[dica] win = %p\n", (void *)win);
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }
    vuln(fd);
    close(fd);
    puts("vuln retornou normalmente (endereco de retorno intacto)");
    return 0;
}
