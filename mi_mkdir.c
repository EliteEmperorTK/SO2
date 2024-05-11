#include "directorios.h"

/**
 *args:
 *argv:
 */
int main(int args, char **argv)
{
    if (args != 4)
    {
        fprintf(stderr, RED "Sintaxis: ./mi_mkdir <nombre_dispositivo> <permisos> </ruta_directorio/> \n" RESET);
        return FALLO;
    }

    const char *nombre_dispositivo = argv[1];
    int permisos = atoi(argv[2]);
    const char *ruta = argv[3];

    if (permisos < 0 || permisos > 7)
    { // Comprobacion de permisos
        fprintf(stderr, RED "Error: modo inválido: <<%d>>\n" RESET, permisos);
        return FALLO;
    }

    if (ruta[atoi(ruta) - 1] == '/')
    {
        mostrar_error_buscar_entrada(ERROR_CAMINO_INCORRECTO);
        return FALLO;
    }

    if (bmount(nombre_dispositivo) == FALLO) // Montamos el dispositivo virtual
    {
        fprintf(stderr, RED "Error al montar el dispositivo virtual.\n" RESET);
        return FALLO;
    }

    // Creamos el directorio
    if (mi_creat(ruta, permisos) == FALLO)
    {

        return FALLO;
    }

    if (bumount() == FALLO) // Desmontamos el dispositivo virtual
    {
        fprintf(stderr, "Error al desmontar el dispositivo virtual.\n");
        exit(FALLO);
    }

    return EXITO;
}
