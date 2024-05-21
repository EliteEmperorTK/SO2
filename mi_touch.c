/* AUTORES:
 * Marc Nadal Sastre Gondar
 * Joaquín Esperon Solari
 * Martí Vich Gispert
 */

#include "directorios.h"

/* Crea un fichero o directorio
 *args: cantidad de argumentos
 *argv: lista de argumentos: [1] = nombre del dispositivo; [2] = permisos que tendrá el directorio; [3] = ruta del directorio a crear
 */
int main(int args, char **argv)
{
    if (args != 4)
    {
        fprintf(stderr, RED "Sintaxis: ./mi_touch <nombre_dispositivo> <permisos> </ruta_directorio/> \n" RESET);
        return FALLO;
    }

    // Almacenamos los parametros
    const char *nombre_dispositivo = argv[1];
    int permisos = atoi(argv[2]);
    const char *ruta = argv[3];

    if (permisos < 0 || permisos > 7)
    { // Comprobacion de permisos
        fprintf(stderr, RED "Error: modo inválido: <<%d>>\n" RESET, permisos);
        return FALLO;
    }

    // se comprueba que la ruta termine en '/'
    if (ruta[strlen(ruta) - 1] == '/') // Verifica si la última posición de ruta es /
    {
        mostrar_error_buscar_entrada(ERROR_CAMINO_INCORRECTO);
        return FALLO;
    }

    if (bmount(nombre_dispositivo) == FALLO) // Montamos el dispositivo virtual
    {
        fprintf(stderr, RED "Error al montar el dispositivo virtual.\n" RESET);
        return FALLO;
    }

    // Creamos el fichero
    if (mi_creat(ruta, permisos) == FALLO)
    {
        // No imprimimos nada porque ya hemos imprimido el error
        bumount();
        return FALLO;
    }

    if (bumount() == FALLO) // Desmontamos el dispositivo virtual
    {
        fprintf(stderr, "Error al desmontar el dispositivo virtual.\n");
        exit(FALLO);
    }

    return EXITO;
}
