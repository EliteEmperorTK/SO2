/* AUTORES:
* Marc Nadal Sastre Gondar
* Joaquín Esperon Solari
* Martí Vich Gispert
*/

#include "directorios.h"

/* Cambia los permisos de un directorio/fichero
 * args: cantidad de argumentos
 * argv: lista de argumentos: [1] = nombre del dispositivo; [2] = permisos a asignar al fichero/directorio; [3] = ruta (camino) del directorio/fichero
 */
int main(int args, char **argv)
{
    if (args != 4)
    {
        fprintf(stderr, RED "Sintaxis: ./mi_chmod <nombre_dispositivo> <permisos> </ruta> \n" RESET);
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

    if (bmount(nombre_dispositivo) == FALLO) // Montamos el dispositivo virtual
    {
        fprintf(stderr, RED "Error al montar el dispositivo virtual.\n" RESET);
        return FALLO;
    }

    if (mi_chmod(ruta, permisos) == FALLO)
    {
        return FALLO;
    }

    if (bumount() == FALLO) // Desmontamos el dispositivo virtual
    {
        fprintf(stderr, RED "Error al desmontar el dispositivo virtual.\n" RESET);
        return FALLO;
    }

    return EXITO;
}
