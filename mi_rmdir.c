/* AUTORES:
 * Marc Nadal Sastre Gondar
 * Joaquín Esperon Solari
 * Martí Vich Gispert
 */

#include "directorios.h"

/* Borra un fichero o directorio
 * args: cantidad de argumentos
 * argv: lista de argumentos: [1] = nombre del dispositivo; [2] = ruta (camino) hasta el fichero/directorio
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

    // Verifica si la última posición de ruta no es /
    if (ruta[strlen(ruta) - 1] != '/')
    {
        mostrar_error_buscar_entrada(ERROR_CAMINO_INCORRECTO);
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