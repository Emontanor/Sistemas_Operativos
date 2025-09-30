#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void *funcion(void *datos){
    printf("Dato recibido: %d\n", *(int *)datos);
}

int main(){
    pthread_t hilo;

    int dato = 1234;
    int r = pthread_create(&hilo,NULL,(void *)funcion,(void *)&dato);
    if(r != 0){
        perror("pthread_create");
        exit(1);
    }

    pthread_join(hilo,NULL);

    return 0;
}