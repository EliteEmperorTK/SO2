/* AUTORES:
 * Marc Nadal Sastre Gondar
 * Joaquín Esperon Solari
 * Martí Vich Gispert
 */

#include "directorios.h"

/* Enseña los nombres de las entradas, además de otros datos del inodo de cada directorio
 *args: cantidad de argumentos
 *argv: lista de argumentos: [1] = nombre del dispositivo; [2] = ruta (camino) hasta el fichero/directorio [3]
 */

int main(int args, char **argv)
{

    char buffer[TAMBUFFER];
    char tipo;

    if (args == 3)
    { // CASO SIN -l

        const char *nombre_dispositivo = argv[1];
        const char *ruta = argv[2];

        if (bmount(nombre_dispositivo) == FALLO) // Montamos el dispositivo virtual
        {
            fprintf(stderr, RED "Error al montar el dispositivo virtual.\n" RESET);
            return FALLO;
        }

        // Si el último elemento es una '/' -> es un directorio
        if (ruta[strlen(ruta) - 1] == '/')
        {
            tipo = 'd';
        }
        else
        {
            tipo = 'f';
        }

        int nEntradas = mi_dir(ruta, buffer, tipo, 0);
        if (nEntradas == FALLO)
        {
            // ya se encarga la otra funcion de imprimir el error
            // fprintf(stderr, RED "Error en mi_dir" RESET);
            return FALLO;
        }

        if (tipo == 'd')
        {
            printf("Total: %d\n", nEntradas);
        }
        printf("%-1s\t%-5s\t%-20s\t%-12s\t%-s\n", "Tipo", "Modo", "mTime", "Tamaño", "Nombre");
        printf("-----------------------------------------------------------\n");
        printf("%s\n", buffer);
    }

    else if (args == 4)
    {
        // CASO -l
        const char *extension = argv[1];

        if (strcmp("-l", extension) != 0)
        {
            fprintf(stderr, RED "Sintaxis introducida incorrecta, sintaxis correcta: './mi_ls -l <disco> </ruta>'" RESET);
            return FALLO;
        }

        const char *nombre_dispositivo = argv[2];
        const char *ruta = argv[3];

        if (bmount(nombre_dispositivo) == FALLO) // Montamos el dispositivo virtual
        {
            fprintf(stderr, RED "Error al montar el dispositivo virtual.\n" RESET);
            return FALLO;
        }

        // Si el último elemento es una '/' -> es un directorio
        if (ruta[strlen(ruta) - 1] == '/')
        {
            tipo = 'd';
        }
        else
        {
            tipo = 'f';
        }

        int nEntradas = mi_dir(ruta, buffer, tipo, 1);
        if (nEntradas == FALLO)
        {
            // ya se encarga la otra funcion de imprimir el error
            // fprintf(stderr, RED "Error en mi_dir" RESET);
            return FALLO;
        }

        if (tipo == 'd')
        {
            printf("Total: %d\n", nEntradas);
        }
        printf("%-1s\t%-5s\t%-20s\t%-12s\t%-s\n", "Tipo", "Modo", "mTime", "Tamaño", "Nombre");
        printf("-----------------------------------------------------------\n");
        printf("%s\n", buffer);
    }
    else
    {
        fprintf(stderr, RED "Sintaxis introducida incorrecta, sintaxis correcta: './mi_ls <disco> </ruta>' \n" RESET);
        return FALLO;
    }

    if (bumount() == FALLO) // Desmontamos el dispositivo virtual
    {
        fprintf(stderr, RED "Error al desmontar el dispositivo virtual.\n" RESET);
        return FALLO;
    }

    return EXITO;
}