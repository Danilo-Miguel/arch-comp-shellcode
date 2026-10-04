#include <fcntl.h>
#include <grp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * Primitiva de corrupcao: variavel adjacente.
 *
 * `name` e `admin` ficam lado a lado na mesma struct, na pilha. A copia abaixo
 * usa o tamanho do arquivo, nao o tamanho de `name`. Se o arquivo tiver mais de
 * 32 bytes, os bytes seguintes caem sobre `admin`.
 *
 * O campo `tail` existe so para absorver bytes extras: este modulo trata apenas
 * da variavel vizinha, nao do endereco de retorno (isso vem no modulo 12).
 */
struct state {
    char name[32];
    volatile unsigned long admin;
    char tail[64];
};

static void drop_privileges(void) {
    if (geteuid() == 0) {
        if (setgroups(0, NULL) != 0 || setgid(1000) != 0 || setuid(1000) != 0) {
            perror("drop privileges");
            exit(1);
        }
    }
}

int main(int argc, char **argv) {
    drop_privileges();
    if (argc != 2) {
        fprintf(stderr, "usage: vuln PAYLOAD\n");
        return 1;
    }

    struct state st;
    memset(&st, 0, sizeof(st));
    st.admin = 0;

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }
    unsigned char data[96];
    ssize_t n = read(fd, data, sizeof(data));
    close(fd);
    if (n <= 0) {
        fprintf(stderr, "payload vazio\n");
        return 1;
    }

    /* A COPIA VULNERAVEL: usa n (tamanho do arquivo), nao sizeof(st.name). */
    memcpy(st.name, data, (size_t)n);

    printf("name = %.32s\n", st.name);
    printf("admin = 0x%lx\n", st.admin);

    if (st.admin == 0x1337UL) {
        puts("VAR_ADJ_OK");
        return 0;
    }
    puts("acesso negado: admin continua 0");
    return 1;
}
