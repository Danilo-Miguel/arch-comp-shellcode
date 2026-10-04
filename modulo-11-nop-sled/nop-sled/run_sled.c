#include <fcntl.h>
#include <grp.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

/*
 * Harness do NOP sled.
 *
 * Em um exploit real voce raramente acerta o endereco exato onde seu shellcode
 * comeca. Para simular essa imprecisao, este harness carrega o payload em memoria
 * executavel e pula para um ponto ALEATORIO perto do inicio (jitter de 0 a
 * JITTER_MAX-1 bytes), em vez de pular exatamente para o byte 0.
 *
 * A defesa do atacante: um NOP sled (uma rampa de 0x90) grande o suficiente antes
 * do shellcode. Caindo em qualquer NOP, a CPU "desliza" ate o shellcode.
 */
#define JITTER_MAX 200

static void drop_privileges(void) {
    if (geteuid() == 0) {
        if (setgroups(0, NULL) != 0 || setgid(1000) != 0 || setuid(1000) != 0) {
            perror("drop privileges");
            exit(1);
        }
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: run_sled PAYLOAD\n");
        return 1;
    }
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }
    off_t length = lseek(fd, 0, SEEK_END);
    if (length <= 0 || length > 4096 || lseek(fd, 0, SEEK_SET) < 0) {
        fprintf(stderr, "payload must contain 1 to 4096 bytes\n");
        return 1;
    }
    unsigned char *memory = mmap(NULL, (size_t)length, PROT_READ | PROT_WRITE | PROT_EXEC,
                                 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (memory == MAP_FAILED) {
        perror("mmap");
        return 1;
    }
    if (read(fd, memory, (size_t)length) != length) {
        perror("read");
        return 1;
    }
    close(fd);

    if (length <= JITTER_MAX) {
        fprintf(stderr, "payload precisa ser maior que o jitter (%d bytes)\n", JITTER_MAX);
        return 1;
    }

    srand((unsigned)(time(NULL) ^ getpid()));
    int jitter = rand() % JITTER_MAX;
    fprintf(stderr, "[harness] pulando para memory+%d\n", jitter);

    drop_privileges();
    void (*entry)(void) = (void (*)(void))(memory + jitter);
    entry();
    return 0;
}
