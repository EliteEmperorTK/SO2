/* AUTORES:
* Marc Nadal Sastre Gondar
* Joaquín Esperon Solari
* Martí Vich Gispert
*/

#include "directorios.h"

/* Crea un enlace a un fichero
 * args: cantidad de argumentos
 * argv: lista de argumentos: [1] = nombre_dispositivo; [2] = ruta (camino) del fichero original; [3] = ruta del enlace
 */
int main(int args, char **argv)
{
    // Validación de argumentos
    if (args != 4)
    {
        fprintf(stderr, RED "Sintaxis: ./mi_link disco /ruta_fichero_original /ruta_enlace\n" RESET);
        return FALLO;
    }

    // Inicialización argumentos
    char *nombre_dispositivo = argv[1];
    char *ruta_fichero_original = argv[2];
    char *ruta_enlace = argv[3];

    if (ruta_fichero_original[strlen(ruta_fichero_original) - 1] == '/')
    { // Se verifica que sea un fichero
        fprintf(stderr, RED "Error: No existe el archivo o el directorio.\n" RESET);
        return FALLO;
    }

    if (ruta_enlace[strlen(ruta_enlace) - 1] == '/')
    { // Se verifica que sea un fichero
        fprintf(stderr, RED "Error: El archivo ya existe.\n" RESET);
        return FALLO;
    }

    // Montamos el dispositivo
    if (bmount(nombre_dispositivo) == FALLO)
    {
        return FALLO;
    }

    // crear enlace a fichero
    if (mi_link(ruta_fichero_original, ruta_enlace) == FALLO)
    {
        return FALLO;
    }

    if (bumount() == FALLO)
    { // Desmontar dispositivo
        return FALLO;
    }
    return EXITO;
}