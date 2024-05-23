/* AUTORES:
 * Marc Nadal Sastre Gondar
 * Joaquín Esperon Solari
 * Martí Vich Gispert
 */

#include "bloques.h"
#include "semaforo_mutex_posix.h"

// Variable global estática para el descriptor del fichero
static int descriptor = 0;

static sem_t *mutex;

static unsigned int inside_sc = 0;

/**
 *   Montar el dispositivo virtual
 *   const char *camino: nombre dispositivo virtual
 */
int bmount(const char *camino)
{

    if (!mutex)
    { // el semáforo es único en el sistema y sólo se ha de inicializar 1 vez (padre)
        mutex = initSem();
        if (mutex == SEM_FAILED)
        {
            return -1;
        }
    }

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
    // Borramos el semáforo
    deleteSem();

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
 *   nbloque: bloque físico a escribir
 *   *buf: contenido a escribir
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
 *   nbloque: bloque físico a leer
 *   *buf: buffer para depositar lo leido
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

// FUNCIONES DEL SEMÁFORO

/**
 *   Llama al semáforo para notificar que se accede a la sección crítica
 */
void mi_waitSem()
{
    if (!inside_sc)
    { // inside_sc==0, no se ha hecho ya un wait
        waitSem(mutex);
    }
    inside_sc++;
}

/**
 *   Llama al semáforo para notificar que se sale de la sección crítica
 */
void mi_signalSem()
{
    inside_sc--;
    if (!inside_sc)
    {
        signalSem(mutex);
    }
}