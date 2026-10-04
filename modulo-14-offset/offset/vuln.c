#include <fcntl.h>
#include <grp.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/*
 * Buffer overflow - etapa 1: descobrir o offset.
 *
 * Igual ao modulo 12 (ret2win), mas o tamanho do buffer NAO e informado. Voce
 * precisa descobrir sozinho quantos bytes existem entre o inicio do buffer e o
 * endereco de retorno. A tecnica e o padrao ciclico (cyclic).
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
    const char msg[] = "OFFSET_OK\n";
    write(1, msg, sizeof(msg) - 1);
    _exit(0);
}

void vuln(int fd) {
    char buf[112];
    read(fd, buf, 400);   /* le muito mais do que cabe */
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
    puts("vuln retornou normalmente");
    return 0;
}
