#include <stdio.h>
#include <stdlib.h>

struct Persona {
    char nombre[30], apellido[30];
    int edad, altura;
};

int main() {
    struct Persona p;
    FILE *archivo;

    printf("Ingrese el nombre: ");
    scanf("%s", p.nombre);
    printf("Ingrese el apellido: ");
    scanf("%s", p.apellido);  
    printf("Ingrese la edad: ");
    scanf("%d", &p.edad);
    printf("Ingrese la altura (en cm): ");
    scanf("%d", &p.altura);

    archivo = fopen("persona.txt", "w");
    if (archivo == NULL) {
        printf("No se pudo abrir el archivo para escribir.\n");
        return 1;
    }
    fprintf(archivo, "%s %s %d %d\n", p.nombre, p.apellido, p.edad, p.altura);
    fclose(archivo);
    printf("Datos guardados en persona.txt\n");

    return 0;
}