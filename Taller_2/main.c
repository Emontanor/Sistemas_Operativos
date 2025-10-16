#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>  // sleep

#define NUM_HILOS 50
pthread_mutex_t mutex;

struct datos_tipo {
    int dato;
    int p;
};

void *proceso(void *datos) {
    pthread_mutex_init(&mutex, NULL);
    struct datos_tipo *datos_proceso = (struct datos_tipo *) datos;
    int a, i, p;

    a = datos_proceso->dato;
    p = datos_proceso->p;

    // ----> sección que muestra el dato p veces
    pthread_mutex_lock(&mutex);
    for (i = 0; i <= p; i++) {
        printf("%i ", a);
    }
    fflush(stdout);

    for (i = 0; i <= p; i++) {
        printf("- ");
    }
    fflush(stdout);
    pthread_mutex_unlock(&mutex);
    // <---- fin de la sección
    return NULL;
}

int main() {
    int error, i;
    // Variables para hilos
    struct datos_tipo hilo_datos[NUM_HILOS];
    pthread_t idhilo[NUM_HILOS];

    for (i = 0; i < NUM_HILOS; i++) {
        hilo_datos[i].dato = i;
        hilo_datos[i].p = i + 1;
    }

    /* se lanzan los hilos */
    for (i = 0; i < NUM_HILOS; i++) {
        error = pthread_create(&idhilo[i], NULL, proceso, (void *)&hilo_datos[i]);
        if (error != 0) {
            perror("No puedo crear hilo");
            exit(-1);
        }
    }

    /* Esperar a que terminen */
    for (i = 0; i < NUM_HILOS; i++) {
        pthread_join(idhilo[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    return 0;
}