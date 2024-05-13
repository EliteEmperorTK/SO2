/* AUTORES:
* Marc Nadal Sastre Gondar
* Joaquín Esperon Solari
* Martí Vich Gispert
*/

#include "ficheros.h"

/* Lee el texto de un inodo
 * args: cantidad de argumentos
 * argv: lista de argumentos =  [1] = nombre del dispositivo; [2] = nº del inodo al que vamos a modificar sus permisos; [3] = nuevos permisos
 */
int main(int args, char **argv)
{
    // Validación sintaxis
    if (args != 4)
    {
        fprintf(stderr, RED "Sintaxis introducida incorrecta, sintaxis correcta: ./permitir <nombre_dispositivo><ninodo><permisos>" RESET);
        return FALLO;
    }
    char *nombre_dispositivo = argv[1];
    int ninodo = atoi(argv[2]);
    int permisos = atoi(argv[3]);

    if (bmount(nombre_dispositivo) == FALLO) // Montamos el dispositivo virtual
    {
        fprintf(stderr, RED "Error en permitir.c al ejecutar bmount()\n" RESET);
        return FALLO;
    }

    if (mi_chmod_f(ninodo, permisos) == FALLO) // Sacamos de los argumentos los permisos y el ninodo
    {
        fprintf(stderr, RED "Error en permitir.c al ejercutar mi_chmod_f()\n" RESET);
        return FALLO;
    }

    if (bumount() == FALLO) // Desmontamos el dispositivo virtual
    {
        fprintf(stderr, RED "Error en permitir.c al ejecutar bumount()\n" RESET);
        return FALLO;
    }

    return EXITO;
}