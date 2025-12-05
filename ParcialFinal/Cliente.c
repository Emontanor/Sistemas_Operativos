#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

long mem_total_kb = 0, mem_free_kb = 0;                                   // Variables de memoria
unsigned long user, nice_time, system, idle, iowait, irq, softirq, steal; // Variables de procesador
char ip[32];                                                              // Ip del servidor
int port;                                                                 // Puerto del servidor
char output_line[256];                                                    // Salida de variables
int fd, r;
char buffer[30];
struct sockaddr_in cliente;

void get_mem();
void get_proc();
void set_message();

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
    if (sscanf(line, "MemFree: %ld kB", &mem_free_kb) == 1)
      continue;
  }
  fclose(f);
}

void get_proc()
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
             &user, &nice_time, &system, &idle,
             &iowait, &irq, &softirq, &steal) == 9)
  {
  }
  else
  {
    fprintf(stderr, "No se pudo leer la línea de cpu\n");
  }
  fclose(f);
}

void set_message()
{
  get_mem();
  get_proc();

  snprintf(output_line, sizeof(output_line), "%lu,%lu,%lu,%lu,%ld,%ld\n", user, nice_time, system, idle, mem_total_kb, mem_free_kb);
}
