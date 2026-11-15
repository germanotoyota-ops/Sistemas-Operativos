#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define N 5
#define COMIDAS 5
#define ARCHIVO "salida_filosofos.txt"

pthread_mutex_t tenedores[N];
pthread_mutex_t mutex_log;
FILE *log_file;

typedef struct {
    int id;
} Filosofo;

void escribir_log(const char *mensaje) {
    pthread_mutex_lock(&mutex_log);
    fprintf(log_file, "%s\n", mensaje);
    fflush(log_file);
    pthread_mutex_unlock(&mutex_log);
}

void pensar(int id) {
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "Filósofo %d está PENSANDO...", id);
    escribir_log(buffer);
    usleep((rand() % 500 + 200) * 1000);
}

void comer(int id) {
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "Filósofo %d está COMIENDO (ñam ñam)", id);
    escribir_log(buffer);
    usleep((rand() % 500 + 200) * 1000);
}

void *filosofo(void *arg) {
    Filosofo *f = (Filosofo *)arg;
    int id = f->id;
    char buffer[128];

    int izq = id;
    int der = (id + 1) % N;

    int primero = izq;
    int segundo = der;
    if (id == N - 1) {
        primero = der;
        segundo = izq;
    }

    for (int i = 0; i < COMIDAS; i++) {
        pensar(id);

        snprintf(buffer, sizeof(buffer),
                 "Filósofo %d intenta tomar tenedor %d", id, primero);
        escribir_log(buffer);
        pthread_mutex_lock(&tenedores[primero]);

        snprintf(buffer, sizeof(buffer),
                 "Filósofo %d intenta tomar tenedor %d", id, segundo);
        escribir_log(buffer);
        pthread_mutex_lock(&tenedores[segundo]);

        snprintf(buffer, sizeof(buffer),
                 "Filósofo %d toma ambos tenedores [%d, %d]",
                 id, primero, segundo);
        escribir_log(buffer);

        comer(id);

        pthread_mutex_unlock(&tenedores[segundo]);
        pthread_mutex_unlock(&tenedores[primero]);

        snprintf(buffer, sizeof(buffer),
                 "Filósofo %d suelta tenedores [%d, %d] y vuelve a pensar",
                 id, primero, segundo);
        escribir_log(buffer);
    }

    snprintf(buffer, sizeof(buffer),
             ">>> Filósofo %d TERMINÓ de comer (%d veces) <<<", id, COMIDAS);
    escribir_log(buffer);

    free(f);
    pthread_exit(NULL);
}

int main(void) {
    pthread_t hilos[N];
    srand(time(NULL));

    log_file = fopen(ARCHIVO, "w");
    if (!log_file) {
        perror("No se pudo abrir el archivo");
        return EXIT_FAILURE;
    }

    escribir_log("=================================================");
    escribir_log("   PROBLEMA DE LA CENA DE LOS FILÓSOFOS");
    escribir_log("=================================================");

    pthread_mutex_init(&mutex_log, NULL);
    for (int i = 0; i < N; i++) {
        pthread_mutex_init(&tenedores[i], NULL);
    }

    for (int i = 0; i < N; i++) {
        Filosofo *f = malloc(sizeof(Filosofo));
        if (!f) { perror("malloc"); exit(EXIT_FAILURE); }
        f->id = i;
        if (pthread_create(&hilos[i], NULL, filosofo, f) != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < N; i++) {
        pthread_join(hilos[i], NULL);
    }

    escribir_log("=================================================");
    escribir_log("   TODOS LOS FILÓSOFOS TERMINARON");
    escribir_log("=================================================");

    for (int i = 0; i < N; i++) {
        pthread_mutex_destroy(&tenedores[i]);
    }
    pthread_mutex_destroy(&mutex_log);
    fclose(log_file);

    printf("Ejecución finalizada. Revisa el archivo '%s'\n", ARCHIVO);
    return EXIT_SUCCESS;
}