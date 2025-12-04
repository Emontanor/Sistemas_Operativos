#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/wait.h>

pid_t pid;
int primos_hijo=0,primos_padre=0;
int cota_superior,cota_inferior,mitad;
key_t key = 1234;

int esPrimo(int numero){
    int divisores = 0;
    int media = (numero/2) + 1;
    for(int i=1;i<media;i++){
        if(numero%i == 0){
            divisores ++;
        }
    }

    if(divisores == 1){
        return 1;
    }else{
        return 0;
    }

}

int main (){

    //inputs
    int cota_superior,cota_inferior;
    printf("Ingrese cota inferior: ");
    scanf("%d",&cota_inferior);
    printf("Ingrese cota superior: ");
    scanf("%d",&cota_superior);
    
    mitad = cota_inferior + ((cota_superior - cota_inferior) / 2);
    //printf("Cotas:[%d,%d]. Punto medio:%d\n",cota_inferior,cota_superior,mitad);

    //creacion de memoria compartida
    int shmId = shmget(key, sizeof(double), 0666|IPC_CREAT);//Crear memoria compartida
    if(shmId == -1){
        perror("shmget");
        exit(1);
    }
    int *ap = shmat(shmId, 0, 0); //Adjuntar memoria compartida
    if(ap == (void*)-1){
        perror("shmat");
        exit(1);
    }

    //lanzamiento de proceso hijo
    pid = fork();
    if(pid < 0){
        perror("Error al crear el fork");
    }else{
        if(pid == 0){
            for(int i=cota_inferior; i<mitad; i++){
                primos_hijo += esPrimo(i);
            }
            *ap = primos_hijo;
            //printf("%d\n",primos_hijo);
        }else{
            wait(NULL);
            for(int i=mitad; i<(cota_superior+1); i++){
                primos_padre += esPrimo(i);
            }
            //printf("%d\n",primos_padre);
            int primos = *ap + primos_padre;
            printf("Primos en el intervalo [%d,%d] = %d\n",cota_inferior,cota_superior,primos);
        }

    }

    shmdt(ap);

    return 0;
}