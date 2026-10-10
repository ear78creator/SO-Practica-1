#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

int recibida = 0;

void manejador_sigusr1(int sig) {
    (void)sig;
    recibida = 1;
}

void manejador_sigalrm(int sig) {
    (void)sig;
}

void proceso_hoja(char nombre, pid_t pid_b, pid_t pid_a, pid_t pid_ejec, int segs, pid_t pid_destino) {
    printf("Soy el proceso %c: mi pid es %d. Mi padre es %d. Mi abuelo es %d. Mi bisabuelo es %d\n",
           nombre, getpid(), pid_b, pid_a, pid_ejec);

    if (nombre == 'Z') {
        signal(SIGALRM, manejador_sigalrm);
        alarm(segs);
        pause();
        kill(pid_destino, SIGUSR1);
        sleep(1);
    } else {
        pause();
    }

    printf("Soy %c (%d) y muero\n", nombre, getpid());
    exit(0);
}

void proceso_b(pid_t pid_a, pid_t pid_ejec, int segs) {
    printf("Soy el proceso B: mi pid es %d. Mi padre es %d. Mi abuelo es %d\n",
           getpid(), pid_a, pid_ejec);

    pid_t pid_x = fork();
    if (pid_x == 0) {
        proceso_hoja('X', getppid(), pid_a, pid_ejec, segs, pid_a);
    }

    pid_t pid_y = fork();
    if (pid_y == 0) {
        proceso_hoja('Y', getppid(), pid_a, pid_ejec, segs, pid_a);
    }

    pid_t pid_z = fork();
    if (pid_z == 0) {
        proceso_hoja('Z', getppid(), pid_a, pid_ejec, segs, pid_a);
    }

    waitpid(pid_z, NULL, 0);
    kill(pid_y, SIGTERM);
    waitpid(pid_y, NULL, 0);
    kill(pid_x, SIGTERM);
    waitpid(pid_x, NULL, 0);

    printf("Soy B (%d) y muero\n", getpid());
    exit(0);
}

void proceso_a(pid_t pid_ejec, int segs) {
    signal(SIGUSR1, manejador_sigusr1);

    printf("Soy el proceso A: mi pid es %d. Mi padre es %d\n", getpid(), pid_ejec);

    pid_t pid_b = fork();
    if (pid_b == 0) {
        proceso_b(getppid(), pid_ejec, segs);
    }

    while (!recibida) {
        pause();
    }

    pid_t pid_pstree = fork();
    if (pid_pstree == 0) {
        execlp("pstree", "pstree", "-c", NULL);
        exit(0);
    }
    waitpid(pid_pstree, NULL, 0);

    waitpid(pid_b, NULL, 0);

    printf("Soy A (%d) y muero\n", getpid());
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        return 1;
    }

    printf("Soy el proceso ejec: mi pid es %d\n", getpid());

    pid_t pid_a = fork();
    if (pid_a == 0) {
        proceso_a(getppid(), atoi(argv[1]));
    }

    waitpid(pid_a, NULL, 0);

    printf("Soy ejec (%d) y muero\n", getpid());
    return 0;
}
