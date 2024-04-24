#include "directorios.h"

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        perror(RED "Sintaxis introducida incorrecta, sintaxis correcta: './mi_stat <disco> </ruta>' \n" RESET);
        return FALLO;
    }

    const char *nombre_dispositivo = argv[1];
    const char *ruta = argv[2];

    if (bmount(nombre_dispositivo) == FALLO) //Montamos el dispositivo virtual
    {
        perror(RED "Error al montar el dispositivo virtual.\n" RESET);
        return FALLO;
    }

    struct STAT stat;

    mi_stat(ruta, &stat);

    if (bumount() == FALLO) //Desmontamos el dispositivo virtual
    {
        perror(RED "Error al desmontar el dispositivo virtual.\n" RESET);
        return FALLO;
    }

    return EXITO;
}