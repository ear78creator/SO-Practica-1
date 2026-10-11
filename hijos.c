#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

void esperar_hijos(int cantidad) {
    int i = 0;
    while (i < cantidad) {
        wait(NULL);
        i++;
    }
}

void imprimir_hoja(int indice, int x, pid_t *pids_padres) {
    printf("Soy el subhijo %d, mi padres son: ", getpid());
    int k = 0;
    while (k < x) {
        printf("%d", pids_padres[k]);
        if (k < x - 1) {
            printf(", ");
        }
        k++;
    }
    printf("\n");
}

void crear_hojas(int y, int x, pid_t *pids_padres, pid_t *pids_hojas) {
    int j = 0;
    while (j < y) {
        pid_t pid = fork();
        if (pid == 0) {
            pids_hojas[j] = getpid();
            imprimir_hoja(j, x, pids_padres);
            shmdt((char *)pids_padres);
            shmdt((char *)pids_hojas);
            exit(0);
        } else if (pid < 0) {
            exit(1);
        }
        j++;
    }
    esperar_hijos(y);
}

void crear_cadena_vertical(int nivel_actual, int x, int y, pid_t *pids_padres, pid_t *pids_hojas) {
    pids_padres[nivel_actual - 1] = getpid();

    if (nivel_actual < x) {
        pid_t pid = fork();
        if (pid == 0) {
            crear_cadena_vertical(nivel_actual + 1, x, y, pids_padres, pids_hojas);
        } else if (pid > 0) {
            wait(NULL);
            shmdt((char *)pids_padres);
            shmdt((char *)pids_hojas);
            exit(0);
        } else {
            exit(1);
        }
    } else {
        crear_hojas(y, x, pids_padres, pids_hojas);
        shmdt((char *)pids_padres);
        shmdt((char *)pids_hojas);
        exit(0);
    }
}

void imprimir_superpadre(int y, pid_t *pids_hojas) {
    printf("Soy el superpadre (%d): mis hijos finales son: ", getpid());
    int j = 0;
    while (j < y) {
        printf("%d", pids_hojas[j]);
        if (j < y - 1) {
            printf(", ");
        }
        j++;
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        return 1;
    }

    int x = atoi(argv[1]);
    int y = atoi(argv[2]);

    int shmid_padres = shmget(IPC_PRIVATE, sizeof(pid_t) * x, IPC_CREAT | 0666);
    int shmid_hojas = shmget(IPC_PRIVATE, sizeof(pid_t) * y, IPC_CREAT | 0666);
    if (shmid_padres < 0 || shmid_hojas < 0) {
        return 1;
    }

    pid_t *pids_padres = (pid_t *)shmat(shmid_padres, NULL, 0);
    pid_t *pids_hojas = (pid_t *)shmat(shmid_hojas, NULL, 0);

    pid_t pid = fork();
    if (pid == 0) {
        crear_cadena_vertical(1, x, y, pids_padres, pids_hojas);
    } else if (pid > 0) {
        wait(NULL);
        imprimir_superpadre(y, pids_hojas);
    }

    shmdt((char *)pids_padres);
    shmdt((char *)pids_hojas);
    shmctl(shmid_padres, IPC_RMID, NULL);
    shmctl(shmid_hojas, IPC_RMID, NULL);

    return 0;
}
