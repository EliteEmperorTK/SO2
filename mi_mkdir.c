#include "directorios.h"

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        perror(RED "Sintaxis introducida incorrecta, sintaxis correcta: './mi_mkdir <disco> <permisos> </ruta>' \n" RESET);
        return FALLO;
    }

    const char *nombre_dispositivo = argv[1];
    int permisos = atoi(argv[2]);
    const char *ruta = argv[3];

    if (permisos < 0 || permisos > 7)
    { // Comprobacion de permisos
        perror(RED "ERROR: Permisos inválidos\n" RESET);
        return FALLO;
    }

    if (ruta[atoi(ruta) - 1] == '/')
    {
        mostrar_error_buscar_entrada(ERROR_CAMINO_INCORRECTO);
        return FALLO;
    }

    if (bmount(nombre_dispositivo) == FALLO)
    {
        perror(RED "Error al montar el dispositivo virtual.\n" RESET);
        return FALLO;
    }
    mi_creat(ruta, permisos);
    if (bumount() == FALLO)
    {
        fprintf(stderr, "Error al desmontar el dispositivo virtual.\n");
        exit(FALLO);
    }

    return EXITO;
}