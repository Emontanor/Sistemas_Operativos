#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/wait.h>

key_t key = 1234;

int main(){

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

    pid_t pid = fork();
    if(pid == 0){
        *ap = 129;
    }else{
        wait(NULL);
        printf("Valor desde el hijo: %d\n", *ap);
    }

    shmdt(ap);

    return 0;
}