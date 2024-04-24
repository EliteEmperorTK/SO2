#include "directorios.h"

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        perror(RED "Sintaxis introducida incorrecta, sintaxis correcta: './mi_ls <disco> </ruta> o ./mi_ls -l <disco> </ruta> #versión extendida' \n" RESET);
        return FALLO;
    }

    const char *nombre_dispositivo = argv[1];
    const char *ruta = argv[2];

    if (bmount(nombre_dispositivo) == FALLO)
    {
        perror(RED "Error al montar el dispositivo virtual.\n" RESET);
        return FALLO;
    }

    char buffer[TAMBUFFER];
    int nEntradas = mi_dir(ruta, buffer);
    if (nEntradas < FALLO)
    {
        perror(RED "Error en mi_dir" RESET);
        return FALLO;
    }

    printf("Total: %d\n", nEntradas);
    printf("%c[%d;%dmTipo\tModo\tmTime\t\t\tTamaño\tNombre%c[%dm\n", 27, 0, 34, 27, 0);
    printf("-----------------------------------------------------------\n");
    printf("%s\n", buffer);

    if (bumount() == FALLO)
    {
        perror(RED "Error al desmontar el dispositivo virtual.\n" RESET);
        return FALLO;
    }

    return EXITO;
}