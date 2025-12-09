#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define BACKLOG 4
#define PORT 9000
#define SHM_NAME "/mi_shm"
#define MAX_ITEMS 100

struct Record {
    char ip[32];
    double cpu_percent;
    unsigned long user;
    unsigned long system;
    unsigned long idle;
    long mem_total_kb;
    long mem_used_kb;
    long mem_available_kb;
    long mem_free_kb;
};

struct ThreadArgs {
    int client_fd;
    int index;
    struct sockaddr_in client_addr;
};

typedef struct {
    struct Record datos[BACKLOG];
    bool ocupado[BACKLOG];
} SharedMemory;

int fd, r, newsock, pid, size;
char bufer[2048];
struct sockaddr_in server, client;
struct Record datos[BACKLOG];
struct ThreadArgs thread_args[BACKLOG];
bool ocupado[BACKLOG];
pthread_mutex_t mutex;
char buffers[4][250];
SharedMemory *shm;

void* manejoCliente(void *args_void);

int main(){
    // Crear socket
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
    {
        perror("socket");
        return 1;
    }

    // Configurar el socket DE SERVIDOR
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;
    bzero(&(server.sin_zero), 8);
    r = bind(fd, (struct sockaddr *)&server, sizeof(struct sockaddr));
    if (r == -1)
    {
        perror("bind");
        return 1;
    }

    r = listen(fd, BACKLOG);
    if (r == -1)
    {
        perror("listen");
        return 1;
    }
    printf("Servidor corriendo\n");

    pthread_mutex_init(&mutex, NULL);
    for (int i = 0; i < BACKLOG; i++) 
    {
        ocupado[i] = false;
    }

    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        return 1;
    }

    ftruncate(shm_fd, sizeof(SharedMemory));

    shm = mmap(
        NULL, 
        sizeof(SharedMemory), 
        PROT_READ | PROT_WRITE, 
        MAP_SHARED, 
        shm_fd, 
        0
    );

    if (shm == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    // Inicializar memoria compartida
    memset(shm, 0, sizeof(SharedMemory));

    while(1)
    {
        size = sizeof(struct sockaddr_in);
        newsock = accept(fd, (struct sockaddr *)&client, &size);
        if (newsock == -1)
        {
            perror("accept");
            return 1;
        }

        for(int i = 0; i < BACKLOG; i++)
        {
            if(!shm->ocupado[i])
            {
                shm->ocupado[i] = true;
                thread_args[i].client_fd = newsock;
                thread_args[i].client_addr = client;
                thread_args[i].index = i;
                pthread_t tid;
                pthread_create(&tid, NULL, manejoCliente, (void*)&thread_args[i]);
                pthread_detach(tid);
                break;
            }
        }
    }

    pthread_mutex_destroy(&mutex);
    close(fd);

    return 0;
    
}

void* manejoCliente(void *args_void)
{
    struct ThreadArgs *args = (struct ThreadArgs*) args_void;

    char ip[32];
    inet_ntop(AF_INET, &args->client_addr.sin_addr, ip, sizeof(ip));
    while (1)
    {
        //printf("Hilo Inicializado");
        r = recv(args->client_fd, buffers[args -> index], 250, 0);
        if (r == 0)
        {
        printf("Cliente desconectado...");
        break;
        }
        if (r < 0)
        {
        perror("Fallo en conexion...");
        break;
        }   
        buffers[args -> index][r] = 0;
        //printf("Datos recibidos de %s: %s\n", ip, buffers[args -> index]);
        struct Record rec;

        //INICIALIZACION DE MUTEX
        pthread_mutex_lock(&mutex);
        strcpy(rec.ip, ip);
        char* token = strtok(buffers[args -> index], ",");
        rec.cpu_percent = atof(token);
        rec.user = strtoul(token, NULL, 10);
        rec.system = strtoul(strtok(NULL, ","), NULL, 10);
        rec.idle = strtoul(strtok(NULL, ","), NULL, 10);
        rec.mem_total_kb = atol(strtok(NULL, ","));
        rec.mem_used_kb = atol(strtok(NULL, ","));
        rec.mem_available_kb = atol(strtok(NULL, ","));
        rec.mem_free_kb = atol(strtok(NULL, ","));
        
        shm -> datos[args -> index] = rec;
        pthread_mutex_unlock(&mutex);
        //FINALIZACION DE MUTEX
    }

    close(args->client_fd);

    // INICIALIZACION DE MUTEX
    pthread_mutex_lock(&mutex);
    shm->ocupado[args->index] = false;
    memset(&shm->datos[args->index], 0, sizeof(struct Record));
    pthread_mutex_unlock(&mutex);
    // FINALIZACION DE MUTEX

    return NULL;
}

