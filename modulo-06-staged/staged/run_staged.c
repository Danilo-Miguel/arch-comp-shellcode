#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static unsigned char *load_payload(const char *path, size_t *length_out) {
    int input = open(path, O_RDONLY);
    if (input < 0) {
        perror("open payload");
        exit(1);
    }
    off_t length = lseek(input, 0, SEEK_END);
    if (length <= 0 || length > 512 || lseek(input, 0, SEEK_SET) < 0) {
        fprintf(stderr, "payload must contain 1 to 512 bytes\n");
        exit(1);
    }
    unsigned char *payload = malloc((size_t)length);
    if (payload == NULL || read(input, payload, (size_t)length) != length) {
        perror("read payload");
        exit(1);
    }
    close(input);
    *length_out = (size_t)length;
    return payload;
}

static void drop_privileges(void) {
    if (geteuid() == 0) {
        if (setgroups(0, NULL) != 0 || setgid(1000) != 0 || setuid(1000) != 0) {
            perror("drop privileges");
            exit(1);
        }
    }
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "usage: run_staged STAGE1 STAGE2\n");
        return 1;
    }
    size_t stage1_length;
    unsigned char *stage1 = load_payload(argv[1], &stage1_length);
    size_t stage2_length;
    unsigned char *stage2 = load_payload(argv[2], &stage2_length);
    int channel[2];
    if (pipe(channel) != 0) {
        perror("pipe");
        return 1;
    }
    pid_t child = fork();
    if (child < 0) {
        perror("fork");
        return 1;
    }
    if (child == 0) {
        close(channel[1]);
        dup2(channel[0], STDIN_FILENO);
        close(channel[0]);
        unsigned char *memory = mmap(NULL, stage1_length,
                                     PROT_READ | PROT_WRITE | PROT_EXEC,
                                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (memory == MAP_FAILED) {
            perror("mmap");
            _exit(1);
        }
        memcpy(memory, stage1, stage1_length);
        drop_privileges();
        void (*shellcode)(void) = (void (*)(void))memory;
        shellcode();
        _exit(1);
    }
    close(channel[0]);
    if (write(channel[1], stage2, stage2_length) != (ssize_t)stage2_length) {
        perror("write stage2");
        return 1;
    }
    close(channel[1]);
    int status;
    waitpid(child, &status, 0);
    return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
}