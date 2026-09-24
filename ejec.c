#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

void manejador_sigusr1(int sig) {
    char comando[64];
    snprintf(comando, sizeof(comando), "pstree -c -p %d", getpid());
    system(comando);
}

void manejador_alarm(int sig) {
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        return 1;
    }

    int segundos = atoi(argv[1]);
    pid_t pid_ejec = getpid();
    printf("Soy el proceso ejec: mi pid es %d\n", pid_ejec);

    pid_t pid_a = fork();
    if (pid_a < 0) {
        return 1;
    }

    if (pid_a == 0) {
        pid_t pid_a_actual = getpid();
        printf("Soy el proceso A: mi pid es %d. Mi padre es %d\n", pid_a_actual, getppid());

        signal(SIGUSR1, manejador_sigusr1);

        pid_t pid_b = fork();
        if (pid_b < 0) {
            exit(1);
        }

        if (pid_b == 0) {
            pid_t pid_b_actual = getpid();
            printf("Soy el proceso B: mi pid es %d. Mi padre es %d. Mi abuelo es %d\n",
                   pid_b_actual, getppid(), pid_ejec);

            pid_t pid_x = fork();
            if (pid_x == 0) {
                printf("Soy el proceso X: mi pid es %d. Mi padre es %d. Mi abuelo es %d. Mi bisabuelo es %d\n",
                       getpid(), getppid(), pid_a_actual, pid_ejec);
                pause();
                printf("Soy X (%d) y muero\n", getpid());
                exit(0);
            }

            pid_t pid_y = fork();
            if (pid_y == 0) {
                printf("Soy el proceso Y: mi pid es %d. Mi padre es %d. Mi abuelo es %d. Mi bisabuelo es %d\n",
                       getpid(), getppid(), pid_a_actual, pid_ejec);
                pause();
                printf("Soy Y (%d) y muero\n", getpid());
                exit(0);
            }

            pid_t pid_z = fork();
            if (pid_z == 0) {
                printf("Soy el proceso Z: mi pid es %d. Mi padre es %d. Mi abuelo es %d. Mi bisabuelo es %d\n",
                       getpid(), getppid(), pid_a_actual, pid_ejec);

                signal(SIGALRM, manejador_alarm);
                alarm(segundos);
                pause();

                kill(pid_a_actual, SIGUSR1);
                sleep(1);

                kill(pid_y, SIGTERM);
                kill(pid_x, SIGTERM);

                printf("Soy Z (%d) y muero\n", getpid());
                exit(0);
            }

            waitpid(pid_z, NULL, 0);
            waitpid(pid_y, NULL, 0);
            waitpid(pid_x, NULL, 0);

            printf("Soy B (%d) y muero\n", getpid());
            exit(0);
        }

        wait(NULL);
        printf("Soy A (%d) y muero\n", getpid());
        exit(0);
    }

    wait(NULL);
    printf("Soy ejec (%d) y muero\n", getpid());

    return 0;
}
