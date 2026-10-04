#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>


#define TOTAL_ITERACIONES 10000
#define ARCHIVO_DESTINO "padre_hijo.txt"

typedef struct {
    int descriptor_archivo;
    const char *prefijo;
    int valor_inicial;
    int valor_final;
    int incremento;
} TareaConcurrente;

static int preparar_archivo(const char *camino);
static void registrar_linea(int descriptor_archivo, const char *prefijo, int numero);
static void procesar_secuencia(const TareaConcurrente *tarea);
static void rutina_proceso_hijo(int descriptor_archivo);
static void rutina_proceso_padre(int descriptor_archivo);

int main(void) {
    int descriptor_archivo = preparar_archivo(ARCHIVO_DESTINO);
    if (descriptor_archivo == -1) {
        return EXIT_FAILURE;
    }

    pid_t identificador_proceso = fork();

    switch (identificador_proceso) {
        case -1:
            perror("fork");
            close(descriptor_archivo);
            return EXIT_FAILURE;

        case 0:
            rutina_proceso_hijo(descriptor_archivo);
            break;

        default:
            rutina_proceso_padre(descriptor_archivo);
            break;
    }

    close(descriptor_archivo);
    return EXIT_SUCCESS;
}

static int preparar_archivo(const char *camino) {
    int descriptor_archivo = open(camino, O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, 0644);
    if (descriptor_archivo == -1) {
        perror("open");
    }
    return descriptor_archivo;
}

static void registrar_linea(int descriptor_archivo, const char *prefijo, int numero) {
    char buffer[64];
    int longitud = sprintf(buffer, "%s %d\n", prefijo, numero);
    if (longitud > 0) {
        write(descriptor_archivo, buffer, (size_t)longitud);
    }
}

static void procesar_secuencia(const TareaConcurrente *tarea) {
    if (tarea->incremento > 0) {
        for (int i = tarea->valor_inicial; i <= tarea->valor_final; i += tarea->incremento) {
            registrar_linea(tarea->descriptor_archivo, tarea->prefijo, i);
        }
    } else if (tarea->incremento < 0) {
        for (int i = tarea->valor_inicial; i >= tarea->valor_final; i += tarea->incremento) {
            registrar_linea(tarea->descriptor_archivo, tarea->prefijo, i);
        }
    }
}

static void rutina_proceso_hijo(int descriptor_archivo) {
    TareaConcurrente tarea_hijo = {
        .descriptor_archivo = descriptor_archivo,
        .prefijo = "HIJO  :",
        .valor_inicial = TOTAL_ITERACIONES,
        .valor_final = 1,
        .incremento = -1
    };
    procesar_secuencia(&tarea_hijo);
}

static void rutina_proceso_padre(int descriptor_archivo) {
    TareaConcurrente tarea_padre = {
        .descriptor_archivo = descriptor_archivo,
        .prefijo = "PADRE :",
        .valor_inicial = 1,
        .valor_final = TOTAL_ITERACIONES,
        .incremento = 1
    };
    procesar_secuencia(&tarea_padre);
    wait(NULL);
}