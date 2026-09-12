#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        return 1;
    }

    int x = atoi(argv[1]);
    int y = atoi(argv[2]);

    if (x <= 0 || y <= 0) {
        return 1;
    }

    for (int j = 1; j <= y; j++) {
        pid_t pid = fork();

        if (pid < 0) {
            return 1;
        }

        if (pid == 0) {
            for (int i = 2; i <= x; i++) {
                pid_t hijo_col = fork();
                if (hijo_col < 0) {
                    exit(1);
                }
                if (hijo_col > 0) {
                    wait(NULL);
                    exit(0);
                }
            }
            sleep(2);
            exit(0);
        }
    }

    sleep(1);
    char comando[64];
    snprintf(comando, sizeof(comando), "pstree -c -p %d", getpid());
    system(comando);

    for (int j = 1; j <= y; j++) {
        wait(NULL);
    }

    return 0;
}
