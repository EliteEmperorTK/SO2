/* AUTORES:
* Marc Nadal Sastre Gondar
* Joaquín Esperon Solari
* Martí Vich Gispert
*/

#include "directorios.h"

/* Muestra la información acerca del inodo de un fichero o directorio
* args: cantidad de argumentos
* argv: lista de argumentos: [1] = nombre del dispositivo; [2] = ruta (camino) hasta el fichero/directorio
*/
int main(int args, char **argv)
{
    if (args != 3)
    {
        fprintf(stderr, RED "Sintaxis introducida incorrecta, sintaxis correcta: './mi_stat <disco> </ruta>' \n" RESET);
        return FALLO;
    }

    const char *nombre_dispositivo = argv[1];
    const char *ruta = argv[2];

    if (bmount(nombre_dispositivo) == FALLO) // Montamos el dispositivo virtual
    {
        fprintf(stderr, RED "Error al montar el dispositivo virtual.\n" RESET);
        return FALLO;
    }

    struct STAT stat;

    if (mi_stat(ruta, &stat) == FALLO)
    {
        return FALLO;
    }

    printf("tipo: %c\n", stat.tipo);
    printf("permisos: %u\n", stat.permisos);
    printf("atime: %s", ctime(&stat.atime));
    printf("ctime: %s", ctime(&stat.ctime));
    printf("mtime: %s", ctime(&stat.mtime));
    printf("nlinks: %u\n", stat.nlinks);
    printf("tamEnBytesLog: %u\n", stat.tamEnBytesLog);
    printf("numBloquesOcupados: %u\n", stat.numBloquesOcupados);

    if (bumount() == FALLO) // Desmontamos el dispositivo virtual
    {
        fprintf(stderr, RED "Error al desmontar el dispositivo virtual.\n" RESET);
        return FALLO;
    }

    return EXITO;
}