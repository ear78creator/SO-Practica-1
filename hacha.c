#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/wait.h>

void hijo_escribir(int tubo_lectura, const char *nombre_archivo, int indice) {
    char sufijo[16];
    char nombre_salida[512];
    int pos = 0;

    sufijo[0] = '.';
    sufijo[1] = 'h';
    sufijo[2] = '0' + (indice / 10);
    sufijo[3] = '0' + (indice % 10);
    sufijo[4] = '\0';

    while (nombre_archivo[pos] != '\0') {
        nombre_salida[pos] = nombre_archivo[pos];
        pos++;
    }

    int k = 0;
    while (sufijo[k] != '\0') {
        nombre_salida[pos] = sufijo[k];
        pos++;
        k++;
    }
    nombre_salida[pos] = '\0';

    int fd_out = creat(nombre_salida, 0666);
    if (fd_out < 0) {
        close(tubo_lectura);
        exit(1);
    }

    char buffer[1024];
    int leidos = read(tubo_lectura, buffer, sizeof(buffer));
    while (leidos > 0) {
        write(fd_out, buffer, leidos);
        leidos = read(tubo_lectura, buffer, sizeof(buffer));
    }

    close(fd_out);
    close(tubo_lectura);
    exit(0);
}

void padre_distribuir(int fd_in, int tam_trozo, int num_trozos, const char *nombre_archivo) {
    int i = 0;
    while (i < num_trozos) {
        int tubo[2];
        if (pipe(tubo) < 0) {
            close(fd_in);
            exit(1);
        }

        pid_t pid = fork();
        if (pid == 0) {
            close(fd_in);
            close(tubo[1]);
            hijo_escribir(tubo[0], nombre_archivo, i);
        }

        close(tubo[0]);

        int bytes_por_enviar = tam_trozo;
        char buffer[1024];
        while (bytes_por_enviar > 0) {
            int a_leer = sizeof(buffer);
            if (bytes_por_enviar < a_leer) {
                a_leer = bytes_por_enviar;
            }

            int leidos = read(fd_in, buffer, a_leer);
            if (leidos <= 0) {
                bytes_por_enviar = 0;
            } else {
                write(tubo[1], buffer, leidos);
                bytes_por_enviar = bytes_por_enviar - leidos;
            }
        }

        close(tubo[1]);
        wait(NULL);
        i++;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        return 1;
    }

    int fd_in = open(argv[1], O_RDONLY);
    if (fd_in < 0) {
        return 1;
    }

    struct stat st;
    if (stat(argv[1], &st) < 0) {
        close(fd_in);
        return 1;
    }

    int tam_trozo = atoi(argv[2]);
    if (tam_trozo <= 0) {
        close(fd_in);
        return 1;
    }

    int num_trozos = st.st_size / tam_trozo;
    if ((st.st_size % tam_trozo) != 0) {
        num_trozos++;
    }

    padre_distribuir(fd_in, tam_trozo, num_trozos, argv[1]);

    close(fd_in);
    return 0;
}
