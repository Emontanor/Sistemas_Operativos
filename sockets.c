#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BACKLOG 4
#define PORT 3535

int fd,r,fd2,pid,size;
char bufer[20];
struct sockaddr_in server,client;

int main(){
    // Crear socket
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(fd == -1){
        perror("socket");
        return 1;
    }

    // Configurar el socket
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;
    bzero (&(server.sin_zero), 8);
    r = bind(fd, (struct sockaddr *)&server, sizeof(struct sockaddr));
    if(r == -1){
        perror("bind");
        return 1;
    }

    listen(fd, BACKLOG);
    if(r == -1){
        perror("listen");
        return 1;
    }

    size = sizeof(struct sockaddr_in);
    fd2 = accept(fd,(struct sockaddr *)&client, &size);
    if(fd2 == -1){
        perror("accept");
        return 1;
    }

    //fork(); // Crear un proceso hijo que atienda al cliente mientras el socket principal sigue escuchando

    r = send(fd2, "Hola cliente\n", 12,0);
    r = recv(fd2, bufer, 20, 0);
    bufer[r] = 0;
    printf("Mensaje: %s\n", bufer);

    close(fd2);
    close(fd);

    return 0;
}