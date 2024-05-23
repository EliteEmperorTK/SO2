/* AUTORES:
 * Marc Nadal Sastre Gondar
 * Joaquín Esperon Solari
 * Martí Vich Gispert
 */

#include "directorios.h"

/* Formatea el dispositivo virtual con el tamaño adecuado de bloques
 * args: cantidad de argumentos
 * argv: lista de argumentos: [1] = nombre del dispositivo; [2] = cantidad de bloques del dispositivo;
 */

int main(int args, char **argv)
{
    if (args != 3)
    {
        fprintf(stderr, "Uso: %s <nombre_dispositivo> <nbloques>\n", argv[0]);
        return FALLO;
    }

    const char *nombre_dispositivo = argv[1];
    unsigned int nbloques = atoi(argv[2]);

    // Montamos el dispositivo virtual
    int descriptor = bmount(nombre_dispositivo);
    if (descriptor == FALLO)
    {
        fprintf(stderr, RED "Error al montar el dispositivo virtual.\n" RESET);
        return FALLO;
    }

    // Inicializar a 0 el fichero utilizado como dispositivo virtual
    unsigned char buffer[BLOCKSIZE];
    memset(buffer, 0, BLOCKSIZE); // Inicializar el buffer a 0s

    for (unsigned int idx = 0; idx < nbloques; idx++)
    {
        if (bwrite(idx, buffer) == FALLO)
        {
            fprintf(stderr, "Error al escribir en el bloque %u.\n", idx);
            bumount();
            return FALLO;
        }
    }

    // Inicializar superbloque
    if (initSB(nbloques, nbloques / 4) == FALLO)
    {
        fprintf(stderr, RED "Error al inicializar el superbloque.\n" RESET);
        bumount();
        return FALLO;
    }

    // Inicializar mapa de bits
    if (initMB() == FALLO)
    {
        fprintf(stderr, RED "Error al inicializar el mapa de bits.\n" RESET);
        bumount();
        return FALLO;
    }

    // Inicializar array de inodos
    if (initAI() == FALLO)
    {
        fprintf(stderr, RED "Error al inicializar el array de inodos.\n" RESET);
        bumount();
        return FALLO;
    }

    // Crear el directorio raíz
    if (reservar_inodo('d', 7) == FALLO)
    {
        fprintf(stderr, RED "Error al crear el directorio raíz.\n" RESET);
        bumount();
        return FALLO;
    }

    if (bumount() == FALLO)
    {
        fprintf(stderr, RED "Error al desmontar el dispositivo virtual.\n" RESET);
        return FALLO;
    }

    // printf("Dispositivo virtual formateado correctamente.\n");

    return EXITO;
}
