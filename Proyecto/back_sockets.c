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
long header[TABLE_SIZE];
FILE *header_file;

void sercher(char *parametros, int fd_out) {
    //abrir archivo necesarios para lectura
    FILE *index_file = fopen("index.dat", "rb");
    if (!index_file) {
        perror("Error al abrir index.dat");
        fclose(header_file);
        return;
    }

    FILE *csv = fopen("final_animedataset.csv", "r");
    if (!csv) {
        perror("Error al abrir final_animedataset.csv");
        fclose(header_file);
        fclose(index_file);
        return;
    }
    
    // --- Procesar parámetros ---
    buffer[0] = 0;
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
        fclose(index_file);
        fclose(csv);
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
        fclose(index_file);
        fclose(csv);
        return;
    } else {
        printf("\nTotal de registros encontrados: %d\n", found);
        snprintf(buffer,sizeof(buffer),"\nTotal de registros encontrados: %d\n", found);
        send(fd_out,buffer,strlen(buffer),0);
        fclose(index_file);
        fclose(csv);
        return;
    }
}

void inserter(char *parametros, int fd_out){
    send(fd_out,"Proceso de insercion para ",26,0);
    send(fd_out,parametros,strlen(parametros),0);
    send(fd_out,"\n",1,0);

    //abrir el csv en modo append
    FILE *temp_csv = fopen("final_animedataset.csv", "a");
    if (!temp_csv) {
        perror("Error al abrir final_animedataset.csv desde el inserter");
    }

    //abrir el index_file en modo append
    FILE *temp_index = fopen("index.dat", "ab");
    if (!temp_index) {
        perror("Error al abrir index.dat desde el inserter");
    }

    long dataset_offset = ftell(temp_csv);
    fprintf(temp_csv, "%s\n", parametros);
    fclose(temp_csv);

    char *new_key = strtok(parametros, ",");
    char key[KEY_SIZE + 1];
    normalize_key(key, new_key);
    unsigned long hash = djb2_hash(key);
    unsigned long bucket = hash % TABLE_SIZE;

    Node new_node;
    new_node.dataset_offset = dataset_offset;
    new_node.next_node_offset = header[bucket];
    
    long new_node_offset = ftell(temp_index);

    fwrite(&new_node, sizeof(Node), 1, temp_index);
    fclose(temp_index);

    header[bucket] = new_node_offset;

    header_file = fopen("header.dat", "wb");
    if (!header_file) {
        perror("Error al abrir header.dat");
    }
    fwrite(header, sizeof(long), TABLE_SIZE, header_file);
    fclose(header_file);

    //Cargar archivos necesarios
    header_file = fopen("header.dat", "rb");
    if (!header_file) {
        perror("Error al abrir header.dat");
        return;
    }

    // --- Cargar header.dat nuevo a memoria ---
    fread(header, sizeof(long), TABLE_SIZE, header_file);
    fclose(header_file);

    send(fd_out,"Insercion completada\n\n",22,0);
    char busqueda[100];
    snprintf(busqueda,sizeof(busqueda), "%s,-,-", new_key);
    sercher(busqueda, fd_out);

    return;
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
    header_file = fopen("header.dat", "rb");
    if (!header_file) {
        perror("Error al abrir header.dat");
        return 1;
    }

    // --- Cargar header.dat completo a memoria ---
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
        
        //recibir el metodo
        r = recv(fd2, buffer, sizeof(buffer), 0);
        if(r == 0){
            printf("Conexion terminada");
            break;
        }else if(r < 0){
            perror("error recv");
            break;
        }else{
            buffer[r] = 0;
            printf("Metodo: %s\n", buffer);
            if (strcmp(buffer, "SEARCH") == 0) {
                send(fd2,"Metodo SEARCH aceptado, esperando parametros:\n",48,0);
                r = recv(fd2, buffer, sizeof(buffer), 0);
                if(r == 0){
                    printf("Conexion terminada");
                    break;
                }else if(r < 0){
                    perror("error recv");
                    break;
                }
                buffer[r] = 0;
                char parametros[100];
                strcpy(parametros, buffer);
                printf("Parametros: %s\n", parametros);
                sercher(parametros, fd2);
                printf("Busqueda finalizada\n\n");
            } else if (strcmp(buffer, "INSERT") == 0) {
                send(fd2,"Metodo INSERT aceptado, esperando parametros:\n",48,0);
                r = recv(fd2, buffer, sizeof(buffer), 0);
                if(r == 0){
                    printf("Conexion terminada");
                    break;
                }else if(r < 0){
                    perror("error recv");
                    break;
                }
                buffer[r] = 0;
                char parametros[100];
                strcpy(parametros, buffer);
                printf("Parametros: %s\n", parametros);
                inserter(parametros, fd2);
                printf("Insercion finalizada\n\n");
            } else {
                send(fd2,"Metodo no soportado",19,0);
                printf("Metodo no soportado\n\n");
            }
            close(fd2);
        }
            
    }     

    close(fd);

    return 0;
}