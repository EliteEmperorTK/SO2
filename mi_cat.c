/* AUTORES:
 * Marc Nadal Sastre Gondar
 * Joaquín Esperon Solari
 * Martí Vich Gispert
 */

#include "directorios.h"

/* Programa que muestra TODO el contenido de un fichero.
 * args: cantidad de argumentos
 * argv: lista de argumentos: [1] = nombre del dispositivo; [2] = ruta (camino) del directorio/fichero;
 */
int main(int args, char **argv)
{
    // Validación de argumentos
    if (args != 3)
    {
        fprintf(stderr, RED "Sintaxis: ./mi_cat <disco> </ruta_fichero>\n" RESET);
        return FALLO;
    }

    // Inicialización argumentos
    char *nombre_dispositivo = argv[1];
    char *ruta_fichero = argv[2];

    struct STAT stat;

    int tambuffer = BLOCKSIZE * 4;
    char buffer_texto[tambuffer];
    memset(buffer_texto, 0, tambuffer);

    int totalLeidos = 0;
    int offset = 0;
    int leidos = 0;

    // Declaramos los punteros
    unsigned int p_entrada = 0;
    unsigned int p_inodo = 0;
    unsigned int p_inodo_dir = 0;

    int posible_error = 0;

    if (bmount(nombre_dispositivo) == FALLO)
    {
        return FALLO;
    }

    if (ruta_fichero[strlen(ruta_fichero) - 1] == '/') // Se verifica que sea un fichero
    {
        fprintf(stderr, "La ruta introducida no es un fichero\n");
        return FALLO;
    }

    // Leemos el fichero especificado recibido como argumento
    leidos = mi_read(ruta_fichero, buffer_texto, offset, tambuffer);

    while (leidos > 0)
    {
        write(1, buffer_texto, leidos);
        totalLeidos += leidos;
        offset += tambuffer;
        memset(buffer_texto, 0, tambuffer);
        leidos = mi_read(ruta_fichero, buffer_texto, offset, tambuffer);
    }

    char string[128];
    sprintf(string, "\nTotal_leidos %d\n", totalLeidos);
    write(2, string, strlen(string));

    // Buscamos la entrada
    posible_error = buscar_entrada(ruta_fichero, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0); // Reservar = 0 porque tan solo consultamos.
    if (posible_error < 0)
    {
        mostrar_error_buscar_entrada(posible_error);
        return posible_error;
    }

    mi_stat_f(p_inodo, &stat);

    if (bumount() == FALLO)
    { // Desmontar dispositivo
        return FALLO;
    }
    return EXITO;
}