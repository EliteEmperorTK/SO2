#include "bloques.h"

int bmount(const char *camino)
{
    int descriptor = open(camino, O_RDWR | O_CREAT, 0666);
    if (descriptor == -1)
    {
        // Error al abrir el fichero
        perror(RED "Error en bmount al abrir el fichero " RESET);
        return FALLO;
    }
    return descriptor;
}
