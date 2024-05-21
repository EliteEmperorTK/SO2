/* AUTORES:
 * Marc Nadal Sastre Gondar
 * Joaquín Esperon Solari
 * Martí Vich Gispert
 */

#include "directorios.h"

int mi_rm_r(char *ruta);

int primera = 0;

/* Función recursica que borra un fichero o directorio no vacío
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

    if (mi_rm_r(ruta) == FALLO)
    {
        bumount();
        return FALLO;
    }

    if (bumount() == FALLO)
    { // Desmontar dispositivo
        return FALLO;
    }
    return EXITO;
}

/* Función recursiva para borrar el contenido de un directorio y el propio directorio
 * ruta: camino hasta el fichero/directorio
 * barra: 0 = no se agrega barra, 1 = se agrega barra
 */
int mi_rm_r(char *ruta)
{

    struct STAT stat;

    if (mi_stat(ruta, &stat) == FALLO)
    {
        fprintf(stderr, RED "Error al obtener el estado de %s\n" RESET, ruta);
        return FALLO;
    }

    if (stat.tipo == 'd')
    {
        // Leer el directorio
        int numentradas = stat.tamEnBytesLog / sizeof(struct entrada);
        struct entrada entradas[numentradas];

        if (mi_read(ruta, entradas, 0, sizeof(entradas)) == FALLO)
        {
            fprintf(stderr, RED "Error al leer el directorio %s\n" RESET, ruta);
            return FALLO;
        }

        // La primera vez no metemos la barra porque ya viene dada en la ruta
        if (primera == 0)
        {
            primera = 1;
        }
        else
        {
            strcat(ruta, "/");
        }

        // Eliminamos todo el contenido del directorio
        for (int i = 0; i < numentradas; i++)
        {
            if (mi_stat(ruta, &stat) == FALLO)
            {
                fprintf(stderr, RED "Error al obtener el estado de %s\n" RESET, ruta);
                return FALLO;
            }

            // Copiamos el valor de la ruta actual en una nueva variable
            char nueva_ruta[1024];
            strcpy(nueva_ruta, ruta);

            // Añadimos el siguiente fichero o directorio
            strcat(nueva_ruta, entradas[i].nombre);

            // Llamamos otra vez a mi_rm_r con la nueva ruta obtenido
            if (mi_rm_r(nueva_ruta) == FALLO)
            {
                return FALLO;
            }
        }

    } // Salimos de la recursividad al encontrar un fichero o al acabar de recorrer todo un directorio -> ya podemos eliminarlo

    // Eliminar el propio fichero/directorio
    if (mi_unlink(ruta) == FALLO)
    {
        fprintf(stderr, YELLOW "Error al borrar %s\n" RESET, ruta);
        return FALLO;
    }

    fprintf(stderr, GREEN "eliminado %s\n" RESET, ruta);
    return EXITO;
}