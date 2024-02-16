#include "bloques.h"

// Variable global estática para el descriptor del fichero
static int descriptor = 0;

int bmount(const char *camino)
{ // Montar el dispositivo virtual
    umask(000);
    descriptor = open(camino, O_RDWR | O_CREAT, 0666);
    if (descriptor == -1)
    {
        // Error al abrir el fichero
        perror(RED "Error en bmount al abrir el fichero " RESET);
        return FALLO;
    }
    return descriptor;
}

int bumount()
{ // Desmonta el dispositivo virtual
    if (close(descriptor) == -1)
    {
        // Error al cerrar el fichero
        perror(RED "Error en bumount al cerrar el fichero" RESET);
        return FALLO;
    }
    return EXITO;
}

int bwrite(unsigned int nbloque, const void *buf)
{
    off_t desplazamiento = nbloque * BLOCKSIZE;
    // O ASI    off_t puntero =lseek(descriptor, desplazamiento, SEEK_SET);
    if (lseek(descriptor, desplazamiento, SEEK_SET) == -1)
    {
        fprintf(stderr, "%sError en bwrite al mover el puntero del fichero: %s%s\n", RED, strerror(errno), RESET);
        return FALLO;
    }

    size_t bytes_escritos = write(descriptor, buf, BLOCKSIZE);
    if (bytes_escritos == -1)
    {
        fprintf(stderr, "%sError en bwrite al escribir en el fichero: %s%s\n", RED, strerror(errno), RESET);
        return FALLO;
    }

    return bytes_escritos;
}

int bread(unsigned int nbloque, void *buf)
{
    off_t desplazamiento = nbloque * BLOCKSIZE;

    if (lseek(descriptor, desplazamiento, SEEK_SET) == -1)
    {
        fprintf(stderr, "%sError en bread al mover el puntero del fichero: %s%s\n", RED, strerror(errno), RESET);
        return FALLO;
    }

    size_t bytes_leidos = read(descriptor, buf, BLOCKSIZE);
    if (bytes_leidos == -1)
    {
        fprintf(stderr, "%sError en bread al leer del fichero: %s%s\n", RED, strerror(errno), RESET);
        return FALLO;
    }

    return bytes_leidos;
}