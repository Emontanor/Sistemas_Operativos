#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>

#define PORT 3535

int fd,r;
bool bandera=true;
char buffer[2048],entrada[100];
struct sockaddr_in cliente;

int main(){
    scanf("%s",entrada);
    while(strcmp(entrada,"exit")!=0){
        //conexion al servidor
        fd = socket(AF_INET,SOCK_STREAM,0);
        if(fd < 0){
            perror("socket");
            return 1;
        }

        //define client socket data
        cliente.sin_family = AF_INET;
        cliente.sin_port = htons(PORT);
        cliente.sin_addr.s_addr = inet_addr("127.0.0.1");
        bzero (&(cliente.sin_zero), 8); 

        //hacer la conexion con el servidor
        r = connect(fd, (struct sockaddr *)&cliente, sizeof(struct sockaddr_in));
        if(r < 0){
            perror("connect");
            return 1;
        }

        //enviar parametros para consulta
        r = send(fd,entrada,sizeof(entrada),0);

        //recibir respuesta
        //recibir_respuesta_prueba(fd, r);
        while(r = recv(fd, buffer, sizeof(buffer), 0)){
            if(r < 0){
                perror("error recv");
                break;
            }else{
                buffer[r] = 0;
                printf("%s",buffer);
            }
        }

        close(fd);
        printf("Conexion cerrada\n\n");
        
        scanf("%s",entrada);
    }
    return 0;
}