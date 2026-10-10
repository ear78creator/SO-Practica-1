#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

void esperar_hijos(int cantidad) {
    int i = 0;
    while (i < cantidad) {
        wait(NULL);
        i++;
    }
}

void crear_columna(int col, int max_filas) {
    int fila = 1;
    while (fila < max_filas) {
        pid_t pid = fork();
        if (pid == 0) {
            fila++;
        } else if (pid > 0) {
            wait(NULL);
            exit(0);
        } else {
            exit(1);
        }
    }
    sleep(15);
    exit(0);
}

void crear_malla(int filas, int columnas) {
    int j = 0;
    while (j < columnas) {
        pid_t pid = fork();
        if (pid == 0) {
            crear_columna(j + 1, filas);
        } else if (pid < 0) {
            exit(1);
        }
        j++;
    }
    execlp("pstree", "pstree", "-c", NULL);
    esperar_hijos(columnas);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        return 1;
    }
    crear_malla(atoi(argv[1]), atoi(argv[2]));
    return 0;
}
