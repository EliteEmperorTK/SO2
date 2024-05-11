#include "directorios.h"

/* borra un fichero o directorio
 * args: cantidad de argumentos
 * argv: lista de argumentos: [1] = nombre_dispositivo; [2] = ruta;
 */
int main(int args, char **argv)
{
    // Validación de argumentos
    if (args != 3)
    {
        fprintf(stderr, RED "Sintaxis: ./mi_rm disco /ruta\n" RESET);
        return FALLO;
    }

    // Inicialización argumentos
    char *nombre_dispositivo = argv[1];
    char *ruta = argv[2];

    // Montamos el dispositivo
    if (bmount(nombre_dispositivo) == FALLO)
    {
        return FALLO;
    }

    // borrar fichero o directorio
    if (mi_unlink(ruta) == FALLO)
    {
        return FALLO;
    }

    if (bumount() == FALLO)
    { // Desmontar dispositivo
        return FALLO;
    }
    return EXITO;
}