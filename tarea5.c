#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define TAM_BUFFER 5
#define NUM_PRODUCTORES 2
#define NUM_CONSUMIDORES 2
#define ITEMS_POR_PRODUCTOR 5

int buffer[TAM_BUFFER];
int entrada = 0;
int salida = 0; 
int contador = 0;  

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t no_lleno = PTHREAD_COND_INITIALIZER;
pthread_cond_t no_vacio = PTHREAD_COND_INITIALIZER;

typedef struct {
    int id;
} ArgsHilo;

void *productor(void *arg) {
    ArgsHilo *args = (ArgsHilo *)arg;
    int id = args->id;

    for (int i = 0; i < ITEMS_POR_PRODUCTOR; i++) {
        int item = (id * 100) + i;

        pthread_mutex_lock(&mutex);
        while (contador == TAM_BUFFER) {
        
            pthread_cond_wait(&no_lleno, &mutex);
        }

        buffer[entrada] = item;
        entrada = (entrada + 1) % TAM_BUFFER;
        contador++;

        printf("Productor %d produce: %d (en buffer: %d)\n", id, item, contador);

        pthread_cond_signal(&no_vacio);
        pthread_mutex_unlock(&mutex);

        usleep(100000);
    }
    return NULL;
}

void *consumidor(void *arg) {
    ArgsHilo *args = (ArgsHilo *)arg;
    int id = args->id;

    int items_a_consumir = (NUM_PRODUCTORES * ITEMS_POR_PRODUCTOR) / NUM_CONSUMIDORES;

    for (int i = 0; i < items_a_consumir; i++) {
        pthread_mutex_lock(&mutex);
        while (contador == 0) {
            pthread_cond_wait(&no_vacio, &mutex);
        }

        int item = buffer[salida];
        salida = (salida + 1) % TAM_BUFFER;
        contador--;

        printf("Consumidor %d consume: %d (en buffer: %d)\n", id, item, contador);

        pthread_cond_signal(&no_lleno);
        pthread_mutex_unlock(&mutex);

        usleep(150000);
    }
    return NULL;
}

int main() {
    if (freopen("salida_productor_consumidor.txt", "w", stdout) == NULL) {
        perror("Error al abrir el archivo de salida");
        return 1;
    }

    pthread_t hilos_p[NUM_PRODUCTORES];
    pthread_t hilos_c[NUM_CONSUMIDORES];
    ArgsHilo args_p[NUM_PRODUCTORES];
    ArgsHilo args_c[NUM_CONSUMIDORES];

    for (int i = 0; i < NUM_PRODUCTORES; i++) {
        args_p[i].id = i + 1;
        if (pthread_create(&hilos_p[i], NULL, productor, &args_p[i]) != 0) {
            perror("Error creando productor");
            exit(1);
        }
    }

    for (int i = 0; i < NUM_CONSUMIDORES; i++) {
        args_c[i].id = i + 1;
        if (pthread_create(&hilos_c[i], NULL, consumidor, &args_c[i]) != 0) {
            perror("Error creando consumidor");
            exit(1);
        }
    }

    for (int i = 0; i < NUM_PRODUCTORES; i++) {
        pthread_join(hilos_p[i], NULL);
    }
    for (int i = 0; i < NUM_CONSUMIDORES; i++) {
        pthread_join(hilos_c[i], NULL);
    }

    printf("Fin de la ejecución. Todos los hilos terminaron.\n");
    fclose(stdout);
    return 0;
}