#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <pthread.h>
#include <semaphore.h>

#define INICIO_SECCION     4000
#define FIN_SECCION        6000
#define ARCHIVO_SALIDA     "salida_semaforos.txt"

sem_t sem_seccion_critica;
pthread_mutex_t mutex_archivo = PTHREAD_MUTEX_INITIALIZER;
FILE *archivo = NULL;

void escribir(const char *formato, ...)
{
    va_list args;
    pthread_mutex_lock(&mutex_archivo);
    va_start(args, formato);
    vfprintf(archivo, formato, args);
    va_end(args);
    fflush(archivo);
    pthread_mutex_unlock(&mutex_archivo);
}

void *contar(void *arg)
{
    int id = *(int *)arg;

    /* Solo cuenta hasta FIN_SECCION (6000) */
    for (int i = 1; i <= FIN_SECCION; i++) {

        /* Al llegar a 4000, intenta entrar a la sección crítica.
         * Si el otro hilo ya está dentro, se bloquea aquí
         * hasta que el otro llegue a 6000 y haga sem_post(). */
        if (i == INICIO_SECCION) {
            escribir("[Hilo %d] Entrando a seccion critica (4000)\n", id);
            sem_wait(&sem_seccion_critica);
            escribir("[Hilo %d] *** DENTRO de seccion critica ***\n", id);
        }

        escribir("Hilo %d -> %d\n", id, i);

        /* Al llegar a 6000, libera el semáforo para que el otro
         * hilo pueda entrar a su sección crítica, y termina. */
        if (i == FIN_SECCION) {
            escribir("[Hilo %d] *** SALIENDO de seccion critica (6000) ***\n", id);
            sem_post(&sem_seccion_critica);
        }
    }

    escribir("[Hilo %d] Fin de la cuenta.\n", id);
    pthread_exit(NULL);
}

int main(void)
{
    pthread_t hilo1, hilo2;
    int id1 = 1, id2 = 2;

    archivo = fopen(ARCHIVO_SALIDA, "w");
    if (archivo == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    if (sem_init(&sem_seccion_critica, 0, 1) != 0) {
        perror("sem_init");
        fclose(archivo);
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&hilo1, NULL, contar, &id1) != 0) {
        perror("pthread_create hilo1");
        fclose(archivo);
        exit(EXIT_FAILURE);
    }
    if (pthread_create(&hilo2, NULL, contar, &id2) != 0) {
        perror("pthread_create hilo2");
        fclose(archivo);
        exit(EXIT_FAILURE);
    }

    pthread_join(hilo1, NULL);
    pthread_join(hilo2, NULL);

    sem_destroy(&sem_seccion_critica);
    pthread_mutex_destroy(&mutex_archivo);

    escribir("\nPrograma finalizado correctamente.\n");
    fclose(archivo);

    printf("Salida generada en el archivo: %s\n", ARCHIVO_SALIDA);
    return 0;
}