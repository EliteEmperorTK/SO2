#include "bloques.h"

// Variable global estática para el descriptor del fichero
static int descriptor = 0;

/**
 *   Montar el dispositivo virtual
 *   const char *camino: nombre dispositivo virtual
 */
int bmount(const char *camino)
{
    umask(000);
    descriptor = open(camino, O_RDWR | O_CREAT, 0666);
    if (descriptor == FALLO)
    {
        // Error al abrir el fichero
        fprintf(stderr, RED "Error en bmount al abrir el fichero." RESET);
        return FALLO;
    }
    return descriptor;
}

/*
 *   Desmonta el dispositivo virtual
 */
int bumount()
{
    if (close(descriptor) == FALLO)
    {
        // Error al cerrar el fichero
        fprintf(stderr, RED "Error en bumount al cerrar el fichero." RESET);
        return FALLO;
    }
    return EXITO;
}

/**
 *   Escribe 1 bloque en el dispositivo virtual
 *   unsigned int nbloque: bloque físico a escribir
 *   const void *buf: contenido a escribir
 */
int bwrite(unsigned int nbloque, const void *buf)
{
    off_t desplazamiento = nbloque * BLOCKSIZE;

    if (lseek(descriptor, desplazamiento, SEEK_SET) == FALLO)
    {
        fprintf(stderr, RED "Error en bwrite al mover el puntero del fichero:\n" RESET);
        return FALLO;
    }

    size_t bytes_escritos = write(descriptor, buf, BLOCKSIZE);
    if (bytes_escritos == FALLO)
    {
        fprintf(stderr, RED "Error en bwrite al escribir en el fichero: \n" RESET);
        return FALLO;
    }

    return bytes_escritos;
}

/**
 *   Lee 1 bloque del dispositivo virtual
 *   unsigned int nbloque: bloque físico a leer
 *   const void *buf: buffer para depositar lo leido
 */
int bread(unsigned int nbloque, void *buf)
{
    off_t desplazamiento = nbloque * BLOCKSIZE;

    if (lseek(descriptor, desplazamiento, SEEK_SET) == FALLO)
    {
        fprintf(stderr, RED "Error en bread al mover el puntero del fichero: \n" RESET);
        return FALLO;
    }

    size_t bytes_leidos = read(descriptor, buf, BLOCKSIZE);
    if (bytes_leidos == FALLO)
    {
        fprintf(stderr, RED "Error en bread al leer del fichero: \n" RESET);
        return FALLO;
    }
    return bytes_leidos;
}
