#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

#define SHM_NAME "/mi_shm"
#define BACKLOG 4

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

typedef struct {
    struct Record datos[BACKLOG];
    bool ocupado[BACKLOG];
} SharedMemory;

SharedMemory *shm;

int main() {
    int shm_fd = shm_open(SHM_NAME, O_RDONLY, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        return 1;
    }

    shm = mmap(
        NULL,
        sizeof(SharedMemory),
        PROT_READ,           // Solo lectura
        MAP_SHARED,
        shm_fd,
        0
    );

    if (shm == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    printf("Visualizador iniciado. Leyendo memoria compartida...\n");

    while (1) {
        system("clear");  // Limpia pantalla 

        printf("===== ESTADO DE CLIENTES =====\n\n");
        printf("IP\t\tCPU%%\tuser\tsystem\tidle\tMemTotal\tMemUsed\tMemAvailable\tMemFree\n");
        printf("---------------------------------------------------------------------------------------------\n");

        for(int i = 0; i < BACKLOG; i++){
            if(shm->ocupado[i]){
                    struct Record rec = shm->datos[i];
                    printf("%s, %lf, %lu, %lu, %lu, %ld, %ld, %ld, %ld\n", 
                        rec.ip,
                        rec.cpu_percent, 
                        rec.user, 
                        rec.system,
                        rec.idle, 
                        rec.mem_total_kb,
                        rec.mem_used_kb,
                        rec.mem_available_kb, 
                        rec.mem_free_kb);
                }
        }

        sleep(2); 
    }

    return 0;
}