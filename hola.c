#include <stdio.h>
#include <stdlib.h>

int main() {
    char *name, *last_name;

    name = malloc(10);
    last_name = malloc(10);

    printf("Write your name: ");
    scanf("%s",name);
    printf("Write your last name: ");
    scanf("%s",last_name);

    printf("%s ",name);
    printf("%s\n ",last_name);

    free(name);
    free(last_name);

    return 0;
}
