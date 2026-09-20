#include <fcntl.h>
#include <grp.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>

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
		fprintf(stderr, "usage: run_payload PAYLOAD\n");
		return 1;
	}
	int input = open(argv[1], O_RDONLY);
	if (input < 0) {
		perror("open payload");
		return 1;
	}
	off_t length = lseek(input, 0, SEEK_END);
	if (length <= 0 || length > 4096 || lseek(input, 0, SEEK_SET) < 0) {
		fprintf(stderr, "payload must contain 1 to 4096 bytes\n");
		return 1;
	}
	unsigned char *memory = mmap(NULL, (size_t)length, PROT_READ | PROT_WRITE | PROT_EXEC,
								 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (memory == MAP_FAILED) {
		perror("mmap");
		return 1;
	}
	ssize_t copied = read(input, memory, (size_t)length);
	close(input);
	if (copied != length) {
		perror("read payload");
		return 1;
	}
	drop_privileges();
	void (*shellcode)(void) = (void (*)(void))memory;
	shellcode();
	return 0;
}