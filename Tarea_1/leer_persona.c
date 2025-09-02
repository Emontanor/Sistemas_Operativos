#include <stdio.h>
#include <stdlib.h>

struct Persona {
    char nombre[30], apellido[30];
    int edad, altura;
};

int main() {
    struct Persona p;
    FILE *archivo;

    archivo = fopen("persona.txt", "r");
    if (archivo == NULL) {
        printf("No se pudo abrir el archivo para leer.\n");
        return 1;
    }
    fscanf(archivo, "%s %s %d %d", p.nombre, p.apellido, &p.edad, &p.altura);
    fclose(archivo);

    printf("Nombre: %s\n", p.nombre);
    printf("Apellido: %s\n", p.apellido);
    printf("Edad: %d\n", p.edad);
    printf("Altura: %d cm\n", p.altura);
    return 0;
}
