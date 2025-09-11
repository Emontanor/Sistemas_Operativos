#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

double pi = 0.0;
double suma_padre = 0.0, suma_hijo = 0.0;

int main()
{
  pid_t pid = fork();

  if (pid < 0)
  {
    printf("Fork Error\n");
    return 1;
  }

  if (pid == 0)
  {
    FILE *sum_hijo;

    for (double i = 0; i < 500000000; i++)
    {
      suma_hijo += 4 / ((4 * i) + 1);
    }

    sum_hijo = fopen("suma-hijo.txt", "w");
    fprintf(sum_hijo, "%lf", suma_hijo);
    fclose(sum_hijo);
  }
  else
  {
    for (double i = 1; i < 500000000; i++)
    {
      suma_padre += 4 / ((4 * i) - 1);
    }

    wait(NULL);

    FILE *archivo;
    archivo = fopen("suma-hijo.txt", "r");
    fscanf(archivo, "%lf", &suma_hijo);
    fclose(archivo);
    
    pi = suma_hijo - suma_padre;
    printf("%.20f\n", pi);
  }

  
  return 0;
}
