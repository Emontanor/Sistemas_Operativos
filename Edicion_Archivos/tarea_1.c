#include <stdio.h>
#include <stdlib.h>

struct Persona{
    char nombre[30], apellido[30];
    int edad, altura;
};

void escribir_persona() {
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
        return;
    }
    fprintf(archivo, "%s %s %d %d\n", p.nombre, p.apellido, p.edad, p.altura);
    fclose(archivo);
    printf("Datos guardados en persona.txt\n");
}

void leer_persona(){
    struct Persona p;
    FILE *archivo;

    archivo = fopen("persona.txt", "r");
    if (archivo == NULL) {
        printf("No se pudo abrir el archivo para leer.\n");
        return;
    }
    fscanf(archivo, "%s %s %d %d", p.nombre, p.apellido, &p.edad, &p.altura);
    fclose(archivo);

    printf("Nombre: %s\n", p.nombre);
    printf("Apellido: %s\n", p.apellido);
    printf("Edad: %d\n", p.edad);
    printf("Altura: %d cm\n", p.altura);
}

int main() {

    char opcion = ' ';

    printf("Que deseas hacer? [w: write, r: read] ");
    scanf(" %c", &opcion);

    if(opcion == 'w'){
        escribir_persona();
    } else if(opcion == 'r'){
        leer_persona();
    }else{
        printf("Opcion no valida\n");
    }

    return 0;
}

//prueba de git console