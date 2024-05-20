/* AUTORES:
 * Marc Nadal Sastre Gondar
 * Joaquín Esperon Solari
 * Martí Vich Gispert
 */

#include "ficheros.h"

/*
 * Escribimos un fichero.
 * ninodo: nº del inodo a escribir
 * buf_original: el buffer que contiene lo que se escribirá
 * offset: posición de escritura inicial con respecto al inodo en bytes lógicos
 * nbytes: tamaño en bytes de buf_original
 */
int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes)
{
    int cant_bytes = 0;
    struct inodo inodo;

    mi_waitSem();

    if (leer_inodo(ninodo, &inodo) == FALLO)
    { // Leemos el inodo
        fprintf(stderr, RED "Error en mi_write_f: No se ha podido leer el inodo.\n" RESET);
        mi_signalSem();
        return FALLO;
    }

    if ((inodo.permisos & 2) != 2)
    { // Miramos si el inodo tiene permisos de escritura
        fprintf(stderr, RED "Error en mi_write_f: No hay permisos de escritura.\n" RESET);
        mi_signalSem();
        return FALLO;
    }

    int primerBL = offset / BLOCKSIZE;                // Primer bloque lógico donde escribimos
    int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE; // Último bloque lógico donde escribimos
    int desp1 = offset % BLOCKSIZE;                   // Desplazamiento con el offset
    int desp2 = (offset + nbytes - 1) % BLOCKSIZE;    // Desplazamiento nbytes después del offset

    int nbfisico = traducir_bloque_inodo(&inodo, primerBL, 1); // Encontramos la posición real (común en ambos casos)
    if (nbfisico == FALLO)
    {
        fprintf(stderr, RED "Error en mi_write_f: No se ha podido encontrar la posición real del primer bloque físico.\n" RESET);
        mi_signalSem();
        return FALLO;
    }

    unsigned char buf_bloque[BLOCKSIZE];
    if (bread(nbfisico, &buf_bloque) == FALLO)
    { // Leemos el bloque y lo metemos en un buffer (común en ambos casos)
        fprintf(stderr, RED "Error en mi_write_f: No se ha podido leer el primer bloque físico. \n" RESET);
        mi_signalSem();
        return FALLO;
    }

    // 2 Casos
    if (primerBL == ultimoBL)
    { // Caso 1 (Escritura en 1 solo bloque)

        memcpy(buf_bloque + desp1, buf_original, nbytes); // Escribimos nbytes del buf_original en la posición (buf_bloque + desp1)
        if (bwrite(nbfisico, buf_bloque) == FALLO)        // Escribimos lo del buffer donde toca
        {
            fprintf(stderr, RED "Error en mi_write_f: No se ha podido escribir en el bloque. \n" RESET);
            mi_signalSem();
            return FALLO;
        };

        cant_bytes = nbytes;
    }
    else
    { // Caso 2 (Escritura en más de 1 bloque)

        // Fase 1: Escribimos el primer bloque
        memcpy(buf_bloque + desp1, buf_original, BLOCKSIZE - desp1);
        if (bwrite(nbfisico, buf_bloque) == FALLO) // Escribimos lo del buffer donde toca
        {
            fprintf(stderr, RED "Error en mi_write_f: No se ha podido escribir en el bloque. \n" RESET);
            mi_signalSem();
            return FALLO;
        };

        cant_bytes = BLOCKSIZE - desp1; // bwrite(nbfisico, buf_bloque);

        // Fase 2: Escribimos los bloques intermedios

        for (int bl = primerBL + 1; bl < ultimoBL; bl++)
        {
            nbfisico = traducir_bloque_inodo(&inodo, bl, 1); // Obtenemos la posición del nuevo bloque
            if (nbfisico == FALLO)
            {
                fprintf(stderr, RED "Error en mi_write_f Caso 2: No se ha podido traducir el bloque de inodo.\n" RESET);
                mi_signalSem();
                return FALLO;
            }

            // Escribimos el bloque nº bl entero
            if (bwrite(nbfisico, buf_original + (BLOCKSIZE - desp1) + (bl - primerBL - 1) * BLOCKSIZE) == FALLO)
            {
                fprintf(stderr, RED "Error en mi_write_f Caso 2: No se ha podido escribir en un bloque intermedio.\n" RESET);
                mi_signalSem();
                return FALLO;
            }

            cant_bytes += BLOCKSIZE; // Aumentamos la cantidad de bytes escritos
        }

        // Fase 3: Escribimos el último bloque
        nbfisico = traducir_bloque_inodo(&inodo, ultimoBL, 1); // Obtenemos la posición del nuevo bloque
        if (nbfisico == FALLO)
        {
            fprintf(stderr, RED "Error en mi_write_f Caso 2 Fase 3: No se ha podido traducir el bloque de inodo.\n" RESET);
            mi_signalSem();
            return FALLO;
        }

        if (bread(nbfisico, &buf_bloque) == FALLO)
        { // Leemos el bloque y lo metemos en un buffer
        }

        memcpy(buf_bloque, buf_original + (nbytes - (desp2 + 1)), desp2 + 1);

        if (bwrite(nbfisico, buf_bloque) == FALLO)
        {
            fprintf(stderr, RED "Error en mi_write_f Caso 2: No se ha podido escribir en el último bloque.\n" RESET);
            mi_signalSem();
            return FALLO;
        }
        cant_bytes += (desp2 + 1);
    }

    // Actualizamos la información del inodo
    if (inodo.tamEnBytesLog < offset + cant_bytes)
    { // Si hemos escrito más allá del tamEnBytesLog, lo actualizamos como nuevo tamaño máximo.
        inodo.tamEnBytesLog = offset + cant_bytes;
        inodo.ctime = time(NULL); // hora actual
    }

    inodo.mtime = time(NULL);

    if (escribir_inodo(ninodo, &inodo) == FALLO)
    { // Escribimos el inodo actualizado
        fprintf(stderr, RED "Error en mi_write_f: No se ha podido escribir el inodo actualizado.\n" RESET);
        mi_signalSem();
        return FALLO;
    }
    // printf("Bytes 6 = %d", cant_bytes);
    mi_signalSem();
    return cant_bytes;
}

/* Leemos un fichero
 * ninodo: nº del inodo a leer
 * buf_original: el buffer en el que se guardará lo leído (debe estar inicializado a 0's)
 * offset: posición de lectura inicial con respecto al inodo en bytes lógicos
 * nbytes: nº de bytes a leer
 */
int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes)
{
    int nbfisico;
    int leidos;
    struct inodo inodo;
    unsigned char buf_bloque[BLOCKSIZE];

    mi_waitSem();

    if (leer_inodo(ninodo, &inodo) == FALLO)
    {
        fprintf(stderr, RED "Error al leer el inodo en mi_read_f" RESET);
        mi_signalSem();
        return FALLO;
    }

    leidos = 0;

    if ((inodo.permisos & 4) != 4)
    {
        // El inodo no tiene permisos de lectura
        fprintf(stderr, RED "El fichero no esta disponible en modo lectura" RESET);
        mi_signalSem();
        return FALLO;
    }

    // Comprobamos que el offset esté dentro del tamaño en bytes lógicos del inodo
    if (offset >= inodo.tamEnBytesLog)
    {
        mi_signalSem();
        return leidos;
    }
    // Si pretende leer más allá de EOF leemos solo hasta el EOF
    if ((offset + nbytes) >= inodo.tamEnBytesLog)
    {
        nbytes = inodo.tamEnBytesLog - offset;
    }

    int primerBL = offset / BLOCKSIZE;                // Primer bloque lógico
    int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE; // Último bloque lógico

    // Calculamos los desplazamientos
    int desp1 = offset % BLOCKSIZE;
    int desp2 = (offset + nbytes - 1) % BLOCKSIZE;

    // Caso 1: el buffer cabe en un solo bloque
    if (primerBL == ultimoBL)
    {
        nbfisico = traducir_bloque_inodo(&inodo, primerBL, 0);
        if (nbfisico > 0)
        {
            if (bread(nbfisico, buf_bloque) < 0)
            {
                fprintf(stderr, RED "Lectura incorrecta del bloque fisico.\n" RESET);
                mi_signalSem();
                return FALLO;
            }
            memcpy(buf_original, buf_bloque + desp1, nbytes);
        }
        leidos = nbytes;
    }
    else
    { // Caso 2: la operación de lectura efecte a más de un bloque
        // Primer bloque: preservar el contenido original del bloque virtual y sobreescribir

        nbfisico = traducir_bloque_inodo(&inodo, primerBL, 0);

        if (nbfisico > 0)
        {
            if (bread(nbfisico, buf_bloque) == FALLO)
            {
                fprintf(stderr, RED "Lectura incorrecta.\n" RESET);
                mi_signalSem();
                return FALLO;
            }
            memcpy(buf_original, buf_bloque + desp1, BLOCKSIZE - desp1);
        }
        leidos = (BLOCKSIZE - desp1);

        // Bloques intermedios: no hay que preservar datos
        for (int bl = primerBL + 1; bl < ultimoBL; bl++)
        {
            nbfisico = traducir_bloque_inodo(&inodo, bl, 0);
            if (nbfisico > 0)
            {
                if (bread(nbfisico, buf_bloque) == FALLO)
                {
                    fprintf(stderr, RED "Error de lectura\n" RESET);
                    mi_signalSem();
                    return FALLO;
                }
                memcpy(buf_original + leidos, buf_bloque, BLOCKSIZE);
            }
            leidos += BLOCKSIZE;
        }

        // Bloque final: preservar la parte restante
        nbfisico = traducir_bloque_inodo(&inodo, ultimoBL, 0);

        if (nbfisico > 0)
        {

            if (bread(nbfisico, buf_bloque) == FALLO)
            {
                fprintf(stderr, RED "Error de lectura\n" RESET);
                mi_signalSem();
                return FALLO;
            }
            memcpy(buf_original + leidos, buf_bloque, desp2 + 1);
        }
        leidos += desp2 + 1;
    }

        // Actualizamos metadatos
    if (leer_inodo(ninodo, &inodo) == FALLO)
    {
        fprintf(stderr, RED "Error al leer el inodo en mi_read_f");
        mi_signalSem();
        return FALLO;
    }

    inodo.atime = time(NULL);

    if (escribir_inodo(ninodo, &inodo) == FALLO)
    {
        fprintf(stderr, RED "Error al escribir el inodo en mi_read_f");
        mi_signalSem();
        return FALLO;
    }
    mi_signalSem();
    return leidos;
}

/* Devuelve la metainformación de un fichero/directorio
 * ninodo: nº inodo del cual vamos a leer su metainformación
 * p_stat: estructura STAT de la qual vamos a leer sus parámetros
 */
int mi_stat_f(unsigned int ninodo, struct STAT *p_stat)
{
    struct inodo inodo;

    // Leer el inodo correspondiente al número de inodo pasado como argumento
    if (leer_inodo(ninodo, &inodo) == FALLO)
    {
        // Error al leer el inodo
        fprintf(stderr, RED "Error en mi_stat_f: No se ha podido leer el inodo.\n" RESET);
        return FALLO;
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

    return EXITO; // Éxito
}

/* Cambia los permisos de un fichero/directorio
 * ninodo: nº inodo al que cambiaremos los permisos
 * permisos: permisos que vamos a conceder
 */
int mi_chmod_f(unsigned int ninodo, unsigned char permisos)
{

    mi_waitSem();
    struct inodo inodo;

    // Leer el inodo correspondiente al número de inodo pasado como argumento
    if (leer_inodo(ninodo, &inodo) == FALLO)
    {
        // Error al leer el inodo
        fprintf(stderr, RED "Error en mi_stat_f: No se ha podido leer el inodo.\n" RESET);
        mi_signalSem();
        return FALLO;
    }

    // Cambiar los permisos del inodo
    inodo.permisos = permisos;
    inodo.ctime = time(NULL); // Actualizar ctime

    // Escribir el inodo modificado en el dispositivo
    if (escribir_inodo(ninodo, &inodo) == FALLO)
    {
        // Error al escribir el inodo modificado
        fprintf(stderr, RED "Error en mi_chmod_f: No se ha podido escribir el inodo.\n" RESET);
        mi_signalSem();
        return FALLO;
    }
    mi_signalSem();
    return EXITO; // Éxito
}

/* Trunca un fichero/directorio
 * ninodo: nº de inodo que se va a truncar
 * nbytes: cantidad de bytes a la que se va a truncar el fichero
 */
int mi_truncar_f(unsigned int ninodo, unsigned int nbytes)
{
    int bloq_liber;
    int primerBL; // Bloque lógico desde el cual empezamos a liberar
    struct inodo inodos;

    if (leer_inodo(ninodo, &inodos) == FALLO)
    { // Leemos el inodo a truncar
        fprintf(stderr, RED "Error al leer el inodo en mi_truncar_f." RESET);
        return FALLO;
    }

    if (!(inodos.permisos & 2))
    { // Comprobamos si tiene permisos de escritura (entra si no tiene)
        fprintf(stderr, RED "El inodo no tiene permisos de escritura en mi_truncar_f." RESET);
        return FALLO;
    }

    // // No podemos truncar más allá del EOF, por tanto
    if (nbytes > inodos.tamEnBytesLog)
    {
        fprintf(stderr, RED "Error: El tamaño del fichero es menor a la cantidad a truncar en mi_truncar_f." RESET);
        return FALLO;
    }

    if ((nbytes % BLOCKSIZE) == 0)
    { // Comprobamos cuál debe ser el primer bloque lógico
        primerBL = nbytes / BLOCKSIZE;
    }
    else
    {
        primerBL = nbytes / BLOCKSIZE + 1;
    }

    bloq_liber = liberar_bloques_inodo(primerBL, &inodos); // Utilizamos la función liberar bloques inodo
    if (bloq_liber == FALLO)
    {
        fprintf(stderr, RED "Error al liberar los bloques del inodo en mi_truncar_f." RESET);
        return FALLO;
    }

    // Actualizamos la información del inodo y lo escribimos
    inodos.ctime = time(NULL); // hora actual
    inodos.mtime = time(NULL);
    inodos.tamEnBytesLog = nbytes;
    inodos.numBloquesOcupados -= bloq_liber; // Restamos los bloques liberados al total de bloques ocupados

    if (escribir_inodo(ninodo, &inodos) == FALLO)
    { // Guardamos el inodo
        fprintf(stderr, RED "Error al escribir el inodo en mi_truncar_f." RESET);
        return FALLO;
    }
    return bloq_liber;
}
