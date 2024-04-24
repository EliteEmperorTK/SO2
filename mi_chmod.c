#include "directorios.h"

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        perror(RED "Sintaxis introducida incorrecta, sintaxis correcta: './mi_chmod <disco> <permisos> </ruta>' \n" RESET);
        return FALLO;
    }

    const char *nombre_dispositivo = argv[1];
    int permisos = atoi(argv[2]);
    const char *ruta = argv[3];

    if (permisos < 0 && permisos > 7)
    { // Comprobacion de permisos
        perror(RED "ERROR: Permisos inválidos\n" RESET);
        return FALLO;
    }

    if (bmount(nombre_dispositivo) == FALLO)
    {
        perror(RED "Error al montar el dispositivo virtual.\n" RESET);
        return FALLO;
    }

    mi_chmod(ruta, permisos);

    if (bumount() == FALLO)
    {
        perror(RED "Error al desmontar el dispositivo virtual.\n" RESET);
        return FALLO;
    }

    return EXITO;
}