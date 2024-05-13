/* AUTORES:
* Marc Nadal Sastre Gondar
* Joaquín Esperon Solari
* Martí Vich Gispert
*/

#include "directorios.h"

/* Permite escribir texto en una posición de un fichero (offset).
 * args: cantidad de argumentos
 * argv: lista de argumentos: [1] = nombre del dispositivo; [2] = ruta (camino) del fichero; [3] = texto; [4] = offset
 */
int main(int args, char **argv)
{
    // Validación de argumentos
    if (args != 5)
    {
        fprintf(stderr, RED "Sintaxis: ./mi_escribir <disco> </ruta_fichero> <texto> <offset>\n" RESET);
        return FALLO;
    }

    // Inicialización argumentos
    char *nombre_dispositivo = argv[1];
    char *ruta_fichero = argv[2];
    char *texto = argv[3];
    unsigned int offset = atoi(argv[4]);

    int bytesEscritos = 0;
    int lon = strlen(texto);
    char buffer[lon];

    // Montamos el dispositivo
    if (bmount(nombre_dispositivo) == FALLO)
    {
        return FALLO;
    }

    if (ruta_fichero[strlen(ruta_fichero) - 1] == '/')
    { // Se verifica que sea un fichero
        fprintf(stderr, "La ruta introducida no es un fichero\n");
        return FALLO;
    }

    strcpy(buffer, texto);
    printf("longitud texto: %d\n", lon);

    bytesEscritos = mi_write(ruta_fichero, &buffer, offset, lon);

    if (bytesEscritos == FALLO)
    {
        printf("Bytes escritos: %d\n", 0);
        return FALLO;
    }
    else
    {
        printf("Bytes escritos: %d\n", bytesEscritos);
    }

    if (bumount() == FALLO)
    { // Desmontar dispositivo
        return FALLO;
    }
    return EXITO;
}