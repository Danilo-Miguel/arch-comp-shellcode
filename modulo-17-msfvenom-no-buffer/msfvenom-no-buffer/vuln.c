#define _GNU_SOURCE
#include <fcntl.h>
#include <grp.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/personality.h>
#include <unistd.h>

/*
 * Buffer overflow - etapa 4: fechar o ciclo.
 *
 * Mesmo harness vulneravel dos modulos 15 e 16. Agora o shellcode injetado nao e
 * escrito a mao: ele vem do msfvenom (ou de um equivalente pwntools, como fallback).
 * Isto liga o bloco de Metasploit (modulos 8-9) ao bloco de buffer overflow.
 *
 * Offset ate o endereco de retorno = 128 + 8 = 136.
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
    read(fd, buf, 400);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: vuln PAYLOAD\n");
        return 1;
    }
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
