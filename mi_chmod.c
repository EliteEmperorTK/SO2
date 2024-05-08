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

    if (permisos < 0 || permisos > 7)
    { // Comprobacion de permisos
        fprintf(stderr, RED "ERROR: Permisos incorrectos en mi_chmod -> permisos = %d\n" RESET, permisos);
        return FALLO;
    }

    if (bmount(nombre_dispositivo) == FALLO) //Montamos el dispositivo virtual
    {
        perror(RED "Error al montar el dispositivo virtual.\n" RESET);
        return FALLO;
    }

    mi_chmod(ruta, permisos);

    if (bumount() == FALLO) //Desmontamos el dispositivo virtual
    {
        perror(RED "Error al desmontar el dispositivo virtual.\n" RESET);
        return FALLO;
    }

    return EXITO;
}
