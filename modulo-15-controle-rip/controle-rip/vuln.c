#define _GNU_SOURCE
#include <fcntl.h>
#include <grp.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/personality.h>
#include <unistd.h>

/*
 * Buffer overflow - etapa 2: controlar o RIP e pular para o proprio buffer.
 *
 * Este programa e compilado com a pilha EXECUTAVEL (-z execstack) e sem ASLR.
 * Para o endereco do buffer ser estavel entre execucoes, o programa se re-executa
 * uma vez desativando a randomizacao (ADDR_NO_RANDOMIZE) e fixando o ambiente.
 * Assim, o endereco vazado em [leak] vale tambem quando o /challenge/check roda.
 *
 * Layout do frame de vuln():
 *   [ buf: 128 ][ rbp salvo: 8 ][ endereco de retorno: 8 ]  -> offset 136
 */

static void drop_privileges(void) {
    if (geteuid() == 0) {
        if (setgroups(0, NULL) != 0 || setgid(1000) != 0 || setuid(1000) != 0) {
            perror("drop privileges");
            exit(1);
        }
    }
}

void vuln(int fd) {
    char buf[128];
    fprintf(stderr, "[leak] buf = %p\n", (void *)buf);
    fflush(stderr);
    read(fd, buf, 400);   /* LEITURA VULNERAVEL */
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: vuln PAYLOAD\n");
        return 1;
    }
    /* Normaliza ambiente e desliga ASLR, re-executando uma unica vez. */
    if (getenv("VULN_READY") == NULL) {
        personality(ADDR_NO_RANDOMIZE);
        char *nenv[] = {"VULN_READY=1", NULL};
        execle("/proc/self/exe", argv[0], argv[1], (char *)NULL, nenv);
        perror("execle");
        return 1;
    }
    drop_privileges();
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }
    vuln(fd);
    close(fd);
    puts("vuln retornou normalmente");
    return 0;
}
