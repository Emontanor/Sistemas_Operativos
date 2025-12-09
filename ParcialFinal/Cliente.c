#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

// Variables de memoria
long mem_total_kb = 0, mem_free_kb = 0, mem_available_kb = 0, mem_used_kb = 0;                

// Variables de procesador t = 0
unsigned long user1, nice_time1, system1, idle1, iowait1, irq1, softirq1, steal1; 

// Variable de procesador t = 1
unsigned long user2, nice_time2, system2, idle2, iowait2, irq2, softirq2, steal2;

// Datos definitivos de cpu
double cpu_percent_global = 0.0;
unsigned long user_global = 0, system_global = 0, idle_global = 0;

char ip[32];           // Ip del servidor
int port;              // Puerto del servidor
char output_line[256]; // Salida de variables

int fd, r;                  //Descriptores de socket
struct sockaddr_in cliente; //Estructura del socket cliente

void get_mem();         //Medicion de memoria
void compute_mem();     //Calculo de uso de memoria

void get_proc_first();  //Medicion de cpu t = 0
void get_proc_second(); //Medicion de cpu t = 1
void compute_cpu();     //Calculo del uso de cpu

void set_message();     //Seteo del mensaje a enviar

int main()
{
  scanf("%s", ip);
  scanf("%d", &port);
  printf("Conexion IP = %s, Puerto = %d\n", ip, port);

  fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0)
  {
    perror("socket");
    return 1;
  }

  // define client socket data
  cliente.sin_family = AF_INET;
  cliente.sin_port = htons(port);
  cliente.sin_addr.s_addr = inet_addr(ip);
  bzero(&(cliente.sin_zero), 8);

  // hacer la conexion con el servidor
  r = connect(fd, (struct sockaddr *)&cliente, sizeof(struct sockaddr_in));
  if (r < 0)
  {
    perror("connect");
    return 1;
  }

  while (1)
  {
    //cpu
    get_proc_first();
    sleep(1);
    get_proc_second();
    compute_cpu();

    //mem
    get_mem();
    compute_mem();

    //create message
    set_message();


    r = send(fd, output_line, strlen(output_line), 0);
    if (r == 0)
    {
      perror("Servidor desconectado...");
      return 0;
    }
    if (r < 0)
    {
      perror("send");
      return 1;
    }
    sleep(2);
  }

  close(fd);

  return 0;
}

void get_mem()
{
  FILE *f = fopen("/proc/meminfo", "r");
  if (!f)
  {
    perror("fopen");
    return;
  }
  char line[256];
  while (fgets(line, sizeof(line), f))
  {
    if (sscanf(line, "MemTotal: %ld kB", &mem_total_kb) == 1)
      continue;
    if (sscanf( line, "MemFree: %ld kB", &mem_free_kb) == 1)
      continue;
    if (sscanf(line, "MemAvailable: %ld kB", &mem_available_kb) == 1)
      continue;
  }
  fclose(f);
}

void compute_mem()
{
  mem_used_kb = mem_total_kb - mem_free_kb;
}

void get_proc_first()
{
  FILE *f = fopen("/proc/stat", "r");
  if (!f)
  {
    perror("fopen");
    return;
  }

  char cpu_label[5];
  if (fscanf(f, "%4s %lu %lu %lu %lu %lu %lu %lu %lu",
             cpu_label,
             &user1, &nice_time1, &system1, &idle1,
             &iowait1, &irq1, &softirq1, &steal1) == 9)
  {
  }
  else
  {
    fprintf(stderr, "No se pudo leer la línea de cpu\n");
  }
  fclose(f);
}

void get_proc_second(){
  FILE *f = fopen("/proc/stat", "r");
  if (!f)
  {
    perror("fopen");
    return;
  }

  char cpu_label[5];
  if (fscanf(f, "%4s %lu %lu %lu %lu %lu %lu %lu %lu",
             cpu_label,
             &user2, &nice_time2, &system2, &idle2,
             &iowait2, &irq2, &softirq2, &steal2) == 9)
  {
  }
  else
  {
    fprintf(stderr, "No se pudo leer la línea de cpu\n");
  }
  fclose(f);
}

void compute_cpu(){
  unsigned long idle_first  = idle1 + iowait1;
  unsigned long idle_second = idle2 + iowait2;

  unsigned long nonidle_first = user1 + nice_time1 + system1 + irq1 + softirq1 + steal1;

  unsigned long nonidle_second = user2 + nice_time2 + system2 + irq2 + softirq2 + steal2;

  unsigned long total_first  = idle_first + nonidle_first;
  unsigned long total_second = idle_second + nonidle_second;

  long total_diff = total_second - total_first;
  long idle_diff  = idle_second - idle_first;

  if (total_diff == 0)
    cpu_percent_global = 0.0;
  else
    cpu_percent_global = 100.0 * (double)(total_diff - idle_diff) / (double)total_diff;
  
  user_global   = user2;
  system_global = system2;
  idle_global   = idle2;  
}

void set_message(){
  snprintf(output_line, sizeof(output_line),
             "%.2f,%lu,%lu,%lu,%ld,%ld,%ld,%ld\n",
             cpu_percent_global,  // porcentaje total de CPU
             user_global,         // tiempo user
             system_global,       // tiempo system
             idle_global,         // tiempo idle
             mem_total_kb,        // MemTotal
             mem_used_kb,         // MemUsed
             mem_available_kb,    // MemAvailable
             mem_free_kb          // MemFree
  );
}

//user: tiempo ejecutando procesos de usuario
//nice: tiempo usuario con prioridad ajustada
//system: tiempo ejecutando kernel
//idle: tiempo ocioso
//iowait: esperando I/O
//irq: interrupciones hardware
//softirq: interrupciones software
//steal: tiempo robado (VM)


//Ip: 18.222.240.226
//Puerto: 9000