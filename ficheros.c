#include "ficheros.h"

int mi_stat_f(unsigned int ninodo, struct STAT *p_stat)
{
    struct inodo inodo;

    // Leer el inodo correspondiente al número de inodo pasado como argumento
    if (leer_inodo(ninodo, &inodo) == -1)
    {
        return -1; // Error al leer el inodo
    }

    // Asignar los valores de los metadatos del inodo al struct STAT pasado por referencia
    p_stat->tipo = inodo.tipo;
    p_stat->permisos = inodo.permisos;
    p_stat->atime = inodo.atime;
    p_stat->mtime = inodo.mtime;
    p_stat->ctime = inodo.ctime;
    p_stat->nlinks = inodo.nlinks;
    p_stat->tamEnBytesLog = inodo.tamEnBytesLog;
    p_stat->numBloquesOcupados = inodo.numBloquesOcupados;

    return 0; // Éxito
}

int mi_chmod_f(unsigned int ninodo, unsigned char permisos)
{
    struct inodo inodo;

    // Leer el inodo correspondiente al número de inodo pasado como argumento
    if (leer_inodo(ninodo, &inodo) == -1)
    {
        return -1; // Error al leer el inodo
    }

    // Cambiar los permisos del inodo
    inodo.permisos = permisos;
    inodo.ctime = time(NULL); // Actualizar ctime

    // Escribir el inodo modificado en el dispositivo
    if (escribir_inodo(ninodo, &inodo) == -1)
    {
        return -1; // Error al escribir el inodo modificado
    }

    return 0; // Éxito
}