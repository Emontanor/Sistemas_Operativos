#include <stdio.h>
#include <unistd.h>

int main(){
    pid_t pid;
    int pipefd[2];

    int respuesta = pipe(pipefd);
    if(respuesta < 0){
        perror("Error al crear el pipe");
    }

    pid = fork();
    if(pid == 0){
        char mensaje_hijo[] = "Hola Mundo\n[Desde proceso hijo]";

        close(pipefd[0]);
        write(pipefd[1], mensaje_hijo, sizeof(mensaje_hijo));
        close(pipefd[1]);
    }else{
        char mensaje[100];

        close(pipefd[1]);
        respuesta = read(pipefd[0],mensaje,sizeof(mensaje));
        close(pipefd[0]);
        mensaje[respuesta] = 0;

        printf("%s\n",mensaje);

    }

    return 0;
}