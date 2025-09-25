#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 3535

int fd,r;
char buffer[30];
struct sockaddr_in cliente;

int main(){
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

    //enviar mensaje
    r = recv(fd,buffer,30,0);
    buffer[r] = 0;
    printf("Mensaje del servidor: %s\n",buffer);

    //recibir mensaje
    r = send(fd,"hola servidor",13,0);
    if(r < 0){
        perror("send");
        return 1;
    }

    close(fd);

    return 0;
}