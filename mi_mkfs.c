#include "ficheros_basico.h"

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "Uso: %s <nombre_dispositivo> <nbloques>\n", argv[0]);
        exit(FALLO);
    }

    const char *nombre_dispositivo = argv[1];
    unsigned int nbloques = atoi(argv[2]);

    int descriptor = bmount(nombre_dispositivo);
    if (descriptor == FALLO)
    {
        fprintf(stderr, "Error al montar el dispositivo virtual.\n");
        exit(FALLO);
    }

    unsigned char buffer[BLOCKSIZE];
    memset(buffer, 0, BLOCKSIZE); // Inicializar el buffer a 0s

    for (unsigned int i = 0; i < nbloques; i++)
    {
        if (bwrite(i, buffer) == FALLO)
        {
            fprintf(stderr, "Error al escribir en el bloque %u.\n", i);
            bumount();
            exit(FALLO);
        }
    }

    if (bumount() == FALLO)
    {
        fprintf(stderr, "Error al desmontar el dispositivo virtual.\n");
        exit(FALLO);
    }

    printf("Dispositivo virtual formateado correctamente.\n");

    return EXITO;
}