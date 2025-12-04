#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

pid_t pid;
int primos_hijo=0,primos_padre=0;
key_t key = 1234;

int esPrimo(int numero){

    return 0;
}

int main (){

     //inputs
    int cota_superior,cota_inferior;
    printf("Ingrese cota inferior: ");
    scanf("%d",&cota_inferior);
    printf("Ingrese cota superior: ");
    scanf("%d",&cota_superior);
    
    //printf("Cotas:[%d,%d]",cota_inferior,cota_superior);

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

    //creacion del proceso hijo
    pid_t pid = fork();
    if(pid == 0){
        //calculos en el proceso hijo
        *ap = primos_hijo;
    }else{
        wait(NULL);
        printf("Valor desde el hijo: %d\n", *ap);
    }

    shmdt(ap);

    return 0;
}