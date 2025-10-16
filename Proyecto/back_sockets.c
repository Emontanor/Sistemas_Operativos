#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "hashmap.h"

#define BACKLOG 4
#define PORT 3535

int fd,r,fd2,pid,size;
char buffer[2048];
struct sockaddr_in server,client;

void sercher(char *parametros, long *header, FILE *index_file, FILE *csv, int fd_out) {
    char *input_key, *criterio_1, *criterio_2;
    input_key = strtok(parametros, ",");
    criterio_1 = strtok(NULL, ",");
    criterio_2 = strtok(NULL, "\0");
    printf("Parametros recibidos: '%s','%s','%s'\n", input_key, criterio_1, criterio_2);

    char key[KEY_SIZE + 1];
    normalize_key(key, input_key);

    // --- Calcular bucket ---
    unsigned long hash = djb2_hash(key);
    unsigned long bucket = hash % TABLE_SIZE;

    // --- Recuperar primer offset desde header ---
    long current_offset = header[bucket];

    if (current_offset == -1) {
        printf("No se encontraron registros para la clave '%s'.\n", key);
        snprintf(buffer,sizeof(buffer),"No se encontraron registros para la clave '%s'.\n", key);
        send(fd_out,buffer,strlen(buffer),0);
        return;
    }

    printf("Buscando registros para '%s' que coincida con '%s','%s'...\n", input_key, criterio_1, criterio_2);
    snprintf(buffer,sizeof(buffer),"Buscando registros para '%s' que coincida con '%s','%s'...\n", input_key, criterio_1, criterio_2);
    send(fd_out,buffer,strlen(buffer),0);

    // --- Recorrer lista enlazada en index.dat ---
    Node node;
    int found = 0;
    
    while (current_offset != -1) {
        fseek(index_file, current_offset, SEEK_SET);
        fread(&node, sizeof(Node), 1, index_file);

        char line[LINE_BUFFER];
        fseek(csv, node.dataset_offset, SEEK_SET);
        if (fgets(line, sizeof(line), csv)) {
            char line_copy[LINE_BUFFER];
            strcpy(line_copy, line);
            char *line_key = strtok(line, ",");
            strtok(NULL, ",");
            strtok(NULL, ",");
            strtok(NULL, ",");
            strtok(NULL, ",");
            char *line_criterio1 = strtok(NULL, ",");
            strtok(NULL,",");
            char *line_criterio2 = strtok(NULL, ",");
            if (strcmp(line_key, input_key) == 0) {
                if ((strcmp(criterio_1, "-") == 0 && strcmp(criterio_2, "-") == 0)) {
                    printf("%s", line_copy);
                    send(fd_out,line_copy,strlen(line_copy),0);
                    found++;
                } else if (strcmp(criterio_1, "-") != 0 && strcmp(criterio_2, "-") == 0) {
                    if (strcmp(line_criterio1, criterio_1) == 0) {
                        printf("%s", line_copy);
                        send(fd_out,line_copy,strlen(line_copy),0);
                        found++;
                    }
                } else if (strcmp(criterio_1, "-") == 0 && strcmp(criterio_2, "-") != 0) {
                    if (strcmp(line_criterio2, criterio_2) == 0) {
                        printf("%s", line_copy);
                        send(fd_out,line_copy,strlen(line_copy),0);
                        found++;
                    }
                } else {
                    if (strcmp(line_criterio1, criterio_1) == 0 && strcmp(line_criterio2, criterio_2) == 0) {
                        printf("%s", line_copy);
                        send(fd_out,line_copy,strlen(line_copy),0);
                        found++;
                    }
                }
            }
        }
        current_offset = node.next_node_offset;
    }

    if (found == 0){
        printf("No se encontraron registros para '%s'.\n", key);
        snprintf(buffer,sizeof(buffer),"No se encontraron registros para '%s'.\n", key);
        send(fd_out,buffer,strlen(buffer),0);
        return;
    } else {
        printf("\nTotal de registros encontrados: %d\n", found);
        snprintf(buffer,sizeof(buffer),"\nTotal de registros encontrados: %d\n", found);
        send(fd_out,buffer,strlen(buffer),0);
        return;
    }
}

int main(){
    // Crear socket
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd == -1){
        perror("socket");
        return 1;
    }

    // Configurar el socket DE SERVIDOR
    int opt = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
        exit(EXIT_FAILURE);
    }

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
    printf("Servidor corriendo\n");

    //fork(); // Crear un proceso hijo que atienda al cliente mientras el socket principal sigue escuchando
    
    //Cargar archivos necesarios
    FILE *header_file = fopen("header.dat", "rb");
    if (!header_file) {
        perror("Error al abrir header.dat");
        return 1;
    }

    FILE *index_file = fopen("index.dat", "rb");
    if (!index_file) {
        perror("Error al abrir index.dat");
        fclose(header_file);
        return 1;
    }

    FILE *csv = fopen("final_animedataset.csv", "r");
    if (!csv) {
        perror("Error al abrir final_animedataset.csv");
        fclose(header_file);
        fclose(index_file);
        return 1;
    }

    // --- Cargar header.dat completo a memoria ---
    long header[TABLE_SIZE];
    fread(header, sizeof(long), TABLE_SIZE, header_file);
    fclose(header_file);
    
    //recibir consultas
    while(1){
        size = sizeof(struct sockaddr_in);
        fd2 = accept(fd,(struct sockaddr *)&client, &size);

        if(fd2 == -1){
            perror("accept");
            return 1;
        }
        printf("Cliente conectado\n");
        
        r = recv(fd2, buffer, sizeof(buffer), 0);
        if(r == 0){
            printf("Conexion terminada");
            break;
        }else if(r < 0){
            perror("error recv");
            break;
        }else{
            buffer[r] = 0;
            printf("Consulta: %s\n", buffer);
            char consulta[100];
            strcpy(consulta, buffer);
            sercher(consulta, header, index_file, csv, fd2);
            close(fd2);
        }
            
    }     

    close(fd);

    return 0;
}