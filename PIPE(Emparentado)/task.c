#include <stdio.h>
#include <unistd.h>

int main(){
    pid_t pid;
    int pipefd[2], respuesta;
    double pi = 0.0, suma_padre = 0.0, suma_hijo = 0.0;


    respuesta = pipe(pipefd);
    if(respuesta <0 ){
        perror("Error al crear la pipe");
        return 1;
    }

    pid = fork();
    if(pid == 0){
        
        double suma_hijo = 0.0;
        for (double i = 0; i < 500000000; i++){
        suma_hijo += 4 / ((4 * i) + 1);
        }

        close(pipefd[0]);
        respuesta = write(pipefd[1], &suma_hijo, 8);
        close(pipefd[1]);
    }else{
        double suma_padre = 0.0;
        for (double i = 1; i < 500000000; i++){
            suma_padre += 4 / ((4 * i) - 1);
        }

        close(pipefd[1]);
        respuesta = read(pipefd[0], &suma_hijo, 8);
        close(pipefd[0]);

        pi = suma_hijo - suma_padre;
        printf("pi: %.20f\n", pi);
    }

    return 0;
}