#include "ficheros.h"

int main(int args, char **argv)
{
    // Validación sintaxis
    if (args != 4)
    {
        fprintf(stderr, RED "Sintaxis introducida incorrecta, sintaxis correcta: ./permitir <nombre_dispositivo><ninodo><permisos>" RESET);
        return -1;
    }
    char *nombre_dispositivo = argv[1];
    int ninodo = atoi(argv[2]);
    int permisos = atoi(argv[3]);
    // montar dispositivo
    if (bmount(nombre_dispositivo) == -1)
    {
        fprintf(stderr, RED "Error en permitir.c al ejecutar bmount()\n" RESET);
        return -1;
    }
    // extraer de los argumentos numero de inodo y permisos del mismo
    if (mi_chmod_f(ninodo, permisos) == -1)
    {
        fprintf(stderr, RED "Error en permitir.c al ejercutar mi_chmod_f()\n" RESET);
        return -1;
    }
    // desmontar dispositivo
    if (bumount() == -1)
    {
        fprintf(stderr, RED "Error en permitir.c al ejecutar bumount()\n" RESET);
        return -1;
    }

    return 0;
}