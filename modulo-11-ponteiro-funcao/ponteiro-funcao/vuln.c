#include <fcntl.h>
#include <grp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * Primitiva de corrupcao: ponteiro de funcao.
 *
 * `handler` fica logo depois de `buf`. O programa sempre chama `handler()` no
 * final. Normalmente aponta para safe(). Se voce estourar `buf`, sobrescreve o
 * ponteiro e a chamada vai para onde voce mandar - por exemplo, para win().
 *
 * Isto ainda nao e o endereco de retorno (modulo 12): e um ponteiro guardado em
 * dados, que o proprio programa decide chamar.
 */
struct handlers {
    char buf[32];
    void (*handler)(void);
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

void safe(void) {
    puts("handler padrao: nada a fazer");
}

void win(void) {
    puts("FUNC_PTR_OK");
}

int main(int argc, char **argv) {
    drop_privileges();
    if (argc != 2) {
        fprintf(stderr, "usage: vuln PAYLOAD\n");
        return 1;
    }

    struct handlers st;
    memset(&st, 0, sizeof(st));
    st.handler = safe;

    fprintf(stderr, "[dica] win = %p\n", (void *)win);

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

    /* COPIA VULNERAVEL: usa n, nao sizeof(st.buf). */
    memcpy(st.buf, data, (size_t)n);

    /* O programa chama o ponteiro - agora possivelmente controlado por voce. */
    st.handler();
    return 0;
}
