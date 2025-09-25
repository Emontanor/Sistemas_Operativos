#include <stdio.h>
#include <unistd.h>

pid_t pid;
long long suma_padre=0, suma_hijo=0, prueba = 2000000000;

int main(){
    pid = fork(); 

    if(pid == 0){
        for(int i = 0 ; i < prueba ; i++){
            suma_hijo += 1;
        }
        printf("Suma del hijo(pid=%d): %lld\n", getpid(), suma_hijo);
    }else{
        for(int i = 0 ; i < prueba ; i++){
            suma_padre += 2;
        }
        printf("Suma del padre(pid=%d): %lld\n", getpid(), suma_padre);
    }

    return 0;
}