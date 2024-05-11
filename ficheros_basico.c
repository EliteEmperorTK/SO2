#include "ficheros_basico.h"

/**
 *   Calcula el tamaño en bloques necesario para el mapa de bits.
 *   nbloques: numero total de bloques
 */
int tamMB(unsigned int nbloques)
{
    int tam = nbloques / 8;
    int temp = tam % BLOCKSIZE;

    if (temp == 0)
    { // La división es exacta
        return (tam / BLOCKSIZE);
    }
    else
    { // La división no es exacta
        return ((tam / BLOCKSIZE) + 1);
    }
}

/**
 *   Calcula el tamaño en bloques del array de inodos.
 *   ninodos: numero total de inodos
 */
int tamAI(unsigned int ninodos)
{
    int tam = ninodos * INODOSIZE;
    int temp = tam % BLOCKSIZE;

    if (temp == 0)
    { // Division exacta
        return (tam / BLOCKSIZE);
    }
    else
    { // Division no exacta
        return ((tam / BLOCKSIZE) + 1);
    }
}

/**
 *   Inicializa los datos del superbloque.
 *   nbloques: numero total de bloques
 *   ninodos: numero total de inodos
 */
int initSB(unsigned int nbloques, unsigned int ninodos)
{ // Superbloques
    struct superbloque SB;
    SB.posPrimerBloqueMB = posSB + tamSB;
    SB.posUltimoBloqueMB = SB.posPrimerBloqueMB + tamMB(nbloques) - 1;
    SB.posPrimerBloqueAI = SB.posUltimoBloqueMB + 1;
    SB.posUltimoBloqueAI = SB.posPrimerBloqueAI + tamAI(ninodos) - 1;
    SB.posPrimerBloqueDatos = SB.posUltimoBloqueAI + 1;
    SB.posUltimoBloqueDatos = nbloques - 1;
    SB.posInodoRaiz = 0;
    SB.posPrimerInodoLibre = 0;

    SB.cantBloquesLibres = nbloques;
    SB.cantInodosLibres = ninodos;

    SB.totBloques = nbloques;
    SB.totInodos = ninodos;

    // Escribir la estructura en el bloque posSB
    if (bwrite(posSB, &SB) == FALLO)
    {
        fprintf(stderr, RED "Error al escribir el superbloque en el disco.\n" RESET);
        return FALLO;
    }

    return EXITO;
}

/**
 *
 *
 *
 */
int block_size(int bytes)
{
    return (bytes + BLOCKSIZE - 1) / BLOCKSIZE;
}

/**
 *   Inicializa el mapa de bits poniendo a 1 los bits que representan los metadatos
 */
int initMB()
{ // Mapa de bits
    struct superbloque SB;

    // Leer el superbloque para obtener información relevante
    if (bread(posSB, &SB) < 0)
    {
        fprintf(stderr, RED "Error al leer el superbloque en el disco.\n" RESET);
        return FALLO;
    }

    // numero de bloques que ocupan los metadatos
    const int nBloquesMetadatos = tamMB(SB.totBloques) + tamAI(SB.totInodos) + tamSB;

    // Calcular el tamaño del mapa de bits necesario en bytes
    // numero de bytes que ocupan los metadatos
    const int nBytesMetadatos = nBloquesMetadatos / 8;
    // numero de bits extra
    const int nExtraBitsMetadatos = nBloquesMetadatos % 8;

    // Calcular el tamaño del bloque del mapa de bits y el tamaño total en bytes
    const int bitmap_block_size = block_size(nBytesMetadatos + (nExtraBitsMetadatos > 0));
    const int bitmap_byte_size = bitmap_block_size * BLOCKSIZE;

    // Crear un buffer para almacenar el mapa de bits en memoria
    unsigned char bitmap[bitmap_byte_size];

    // Llenar los bytes correspondientes a los metadatos con 1s
    memset(bitmap, 255, nBytesMetadatos);

    // Llenar el último byte parcial con 1s y 0s según los bits restantes de los metadatos
    memset(bitmap + nBytesMetadatos + 1, 0, bitmap_byte_size - nBytesMetadatos);
    bitmap[nBytesMetadatos] = 255 << (8 - nExtraBitsMetadatos);

    // Escribir el mapa de bits en el disco
    for (int i = 0; i < bitmap_block_size; ++i)
    {
        if (bwrite(SB.posPrimerBloqueMB + i, &bitmap[i * BLOCKSIZE]) == FALLO)
        {
            fprintf(stderr, RED "Error al escribir un bloque en initMB");
            return FALLO;
        }
    }

    // Actualizar la cantidad de bloques libres en el superbloque
    SB.cantBloquesLibres -= nBloquesMetadatos;

    // Escribir el superbloque actualizado en el disco
    if (bwrite(posSB, &SB) == FALLO)
    {
        fprintf(stderr, RED "Error al escribir el superbloque actualizado en initMB" RESET);
    }

    return EXITO;
}

/**
 *   inicializar la lista de inodos libres
 */
int initAI()
{ // Array Inodos
    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO)
    {
        fprintf(stderr, RED "Error al leer el superbloque en el disco.\n" RESET);
        return FALLO;
    }

    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    unsigned int contInodos = SB.posPrimerInodoLibre + 1;

    for (int idx = SB.posPrimerBloqueAI; idx <= SB.posUltimoBloqueAI; idx++)
    { // Para cada bloque del array de inodos

        // leer el bloque de inodos en el dispositivo virtual
        if (bread(idx, inodos) == FALLO)
        {
            fprintf(stderr, RED "Error al leer el bloque de inodos en el disco.\n" RESET);
            return FALLO;
        }

        // Inicializar cada inodo del bloque
        for (int i = 0; i < BLOCKSIZE / INODOSIZE; i++)
        {                         // Para cada inodo del bloque
            inodos[i].tipo = 'l'; // libre

            if (contInodos < SB.totInodos)
            {                                               // Si no hemos llegado al último inodo del array de inodos
                inodos[i].punterosDirectos[0] = contInodos; // Enlazamos con el siguiente inodo libre
                contInodos++;
            }
            else
            { // Hemos llegado al último inodo
                inodos[i].punterosDirectos[0] = UINT_MAX;
                break;
            }
        }
        // Escribir el bloque de inodos en el dispositivo virtual
        if (bwrite(idx, inodos) == FALLO)
        {
            fprintf(stderr, RED "Error al leer el superbloque en el disco.\n" RESET);
            return FALLO;
        }
    }
    return EXITO;
}

// NIVEL 3

/**
 *   Escribe el valor indicado por bit en un bloque del MB
 *   nbloque: nº de bloque a escribir
 *   bit: valor 0 o 1 a escribir
 */
int escribir_bit(unsigned int nbloque, unsigned int bit)
{
    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO)
    {
        fprintf(stderr, RED "Error al leer el superbloque.\n" RESET);
        return FALLO;
    }

    int posbyte = nbloque / 8;
    int posbit = nbloque % 8;
    int nbloqueMB = posbyte / BLOCKSIZE;
    int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;
    unsigned char bufferMB[BLOCKSIZE];
    if (bread(nbloqueabs, bufferMB) == FALLO)
    {
        fprintf(stderr, RED "Error al leer el bloque de datos en el disco.\n" RESET);
        return FALLO;
    }
    posbyte = posbyte % BLOCKSIZE;
    unsigned char mascara = 128;
    mascara >>= posbit;
    if (bit == 1)
    {
        bufferMB[posbyte] |= mascara;
    }
    else
    {
        bufferMB[posbyte] &= ~mascara;
    }

    if (bwrite(nbloqueabs, bufferMB) == FALLO)
    {
        fprintf(stderr, RED "Error al escribir el bit" RESET);
        return FALLO;
    }
    return EXITO;
}

/**
 *   Lee un determinado bit del MB y devuelve el valor del bit leído.
 *   nbloque: nº de bloque a leer el bit
 */
char leer_bit(unsigned int nbloque)
{
    // Leer el superbloque para obtener la localización del MB.
    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO)
    {
        fprintf(stderr, RED "Error al leer el superbloque.\n" RESET);
        return FALLO;
    }

    int posbyte = nbloque / 8;
    int posbit = nbloque % 8;
    int nbloqueMB = posbyte / BLOCKSIZE;
    int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;
    unsigned char bufferMB[BLOCKSIZE];
    if (bread(nbloqueabs, bufferMB) == FALLO)
    {
        fprintf(stderr, RED "Error al leer el bloque de datos en el disco.\n" RESET);
        return FALLO;
    }
    posbyte = posbyte % BLOCKSIZE;
    unsigned char mascara = 128;  // 10000000
    mascara >>= posbit;           // desplazamiento de bits a la derecha, los que indique posbit
    mascara &= bufferMB[posbyte]; // operador AND para bits
    mascara >>= (7 - posbit);     // desplazamiento de bits a la derecha para dejar el 0 o 1 en el extremo derecho y leerlo en decimal
    return mascara;
}

/**
 *   Encuentra el primer bloque libre, consultando el MB (primer bit a 0),
 *   lo ocupa (poniendo el correspondiente bit a 1 con la ayuda de la función escribir_bit()) y devuelve su posición.
 */
int reservar_bloque()
{
    struct superbloque SB;
    // Leer el superbloque
    if (bread(posSB, &SB) == FALLO)
    {
        fprintf(stderr, RED "Error al leer el superbloque en el disco.\n" RESET);
        return FALLO;
    }

    unsigned char bufferMB[BLOCKSIZE];
    int nbloqueMB;
    int posbyte = 0;
    int posbit = 0;
    int nbloque = 0;

    // Comprobar si quedan bloques libres
    if (SB.cantBloquesLibres <= 0)
    {
        fprintf(stderr, RED "No quedan bloques libres en el disco.\n" RESET);
        return FALLO;
    }

    // Iterar sobre los bloques del mapa de bits
    for (nbloqueMB = 0; nbloqueMB <= (SB.posUltimoBloqueMB - SB.posPrimerBloqueMB); nbloqueMB++)
    {
        // Leer el bloque del mapa de bits
        if (bread(nbloqueMB + SB.posPrimerBloqueMB, bufferMB) == FALLO)
        {
            fprintf(stderr, RED "Error al leer un bloque del mapa de bits en reservar_bloque.\n" RESET);
            return FALLO;
        }

        //   Comparar con el buffer auxiliar para encontrar el primer byte con algún bit a 0
        unsigned char bufferAux[BLOCKSIZE];
        memset(bufferAux, 255, BLOCKSIZE); // llenamos el buffer auxiliar con bits a 1

        if (memcmp(bufferMB, bufferAux, BLOCKSIZE) != 0)
        {
            break;
        }
    }

    // Encontrar el primer byte con algún bit a 0
    for (posbyte = 0; posbyte < BLOCKSIZE; posbyte++)
    {
        if (bufferMB[posbyte] != 255)
        {
            break;
        }
    }

    // Encontrar el primer bit a 0 dentro de ese byte
    unsigned char mascara = 128;        // 10000000
    while (bufferMB[posbyte] & mascara) // operador AND para bits
    {
        bufferMB[posbyte] <<= 1; // desplazamiento de bits a la izquierda
        posbit++;
    }

    // Calcular el número de bloque físico
    nbloque = (nbloqueMB * BLOCKSIZE + posbyte) * 8 + posbit;

    // Marcar el bloque como reservado
    if (escribir_bit(nbloque, 1) == FALLO)
    {
        fprintf(stderr, RED "Error al escribir un bit en reservar_bloque.\n" RESET);
        return FALLO;
    }

    // Actualizar la cantidad de bloques libres en el superbloque
    SB.cantBloquesLibres--;

    // Guardar el superbloque actualizado
    if (bwrite(posSB, &SB) == FALLO)
    {
        fprintf(stderr, RED "Error al guardar el superbloque actualizado en el disco.\n" RESET);
        return FALLO;
    }

    // Limpiar el bloque en la zona de datos
    unsigned char bufferDatos[BLOCKSIZE];
    memset(bufferDatos, 0, BLOCKSIZE);
    if (bwrite(nbloque, bufferDatos) == FALLO)
    {
        fprintf(stderr, RED "Error al limpiar el bloque %d en la zona de datos.\n" RESET, nbloque);
        return FALLO;
    }

    // Devolver el número de bloque reservado
    return nbloque;
}

/**
 *   Libera un bloque determinado
 *   nbloque: bloque a liberar
 */
int liberar_bloque(unsigned int nbloque)
{
    struct superbloque SB;
    // Leer el superbloque para obtener información relevante
    if (bread(posSB, &SB) == FALLO)
    {
        fprintf(stderr, RED "Error al leer el superbloque en el disco.\n" RESET);
        return FALLO;
    }

    if (escribir_bit(nbloque, 0) == FALLO)
    {
        fprintf(stderr, RED "Error al escribir el bit en la posición %d.\n" RESET, nbloque);
        return FALLO;
    }

    // Incrementar la cantidad de bloques libres en el superbloque
    SB.cantBloquesLibres++;

    // Escribir el superbloque actualizado en el dispositivo
    if (bwrite(posSB, &SB) == FALLO)
    {
        fprintf(stderr, RED "Error al escribir el superbloque en el disco.\n" RESET);
        return FALLO;
    }

    // Devolver el número de bloque liberado
    return nbloque;
}

/*
 *   Escribe el contenido de una variable de tipo struct inodo, pasada por referencia,
 *   en un determinado inodo del array de inodos, inodos.
 *   ninodo: numero total de inodos
 *   inodo: inodo a escribir
 */
int escribir_inodo(unsigned int ninodo, struct inodo *inodo)
{
    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO)
    {
        fprintf(stderr, RED "Error en escribir_inodo al leer el superbloque." RESET);
        return FALLO;
    }

    // Encontramos el bloque
    int nbloqueAI = (ninodo * INODOSIZE) / BLOCKSIZE;
    int nbloqueAbs = nbloqueAI + SB.posPrimerBloqueAI;

    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    if (bread(nbloqueAbs, inodos) == FALLO)
    {
        // Error al leer el bloque
        fprintf(stderr, RED "Error en escribir_inodo al leer el bloque." RESET);
        return FALLO;
    }

    int posinodo = ninodo % (BLOCKSIZE / INODOSIZE);
    inodos[posinodo] = *inodo;

    if (bwrite(nbloqueAbs, inodos) == FALLO)
    {
        // Error al escribir el bloque
        fprintf(stderr, RED "Error en escribir_inodo al escribir el bloque." RESET);
        return FALLO;
    }
    return EXITO;
}

/**
 *Lee un determinado inodo del array de inodos para volcarlo en
 *una variable de tipo struct inodo pasada por referencia.
 * ninodo: numero total de inodos
 * inodo: inodo donde se almacena lo leido
 */
int leer_inodo(unsigned int ninodo, struct inodo *inodo)
{
    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO)
    {
        fprintf(stderr, RED "Error en leer_inodo al leer el superbloque." RESET);
        return FALLO;
    }

    // Encontramos el bloque
    int nbloqueAI = (ninodo * INODOSIZE) / BLOCKSIZE;
    int nbloqueAbs = nbloqueAI + SB.posPrimerBloqueAI;

    struct inodo inodos[BLOCKSIZE / INODOSIZE];

    if (bread(nbloqueAbs, inodos) == FALLO)
    {
        // Error al leer el bloque
        fprintf(stderr, RED "Error en escribir_inodo al leer el bloque." RESET);
        return FALLO;
    }

    int posinodo = ninodo % (BLOCKSIZE / INODOSIZE);
    *inodo = inodos[posinodo];
    return EXITO;
}

/**
 *Encuentra el primer inodo libre,lo reserva, devuelve su número y
 * actualiza la lista enlazada de inodos libres.
 *   tipo: tipo del inodo a reservar
 *   permisos:  permisos del inodo a reservar
 */
int reservar_inodo(unsigned char tipo, unsigned char permisos)
{
    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO)
    {
        fprintf(stderr, RED "Error en reservar_inodo al leer el superbloque." RESET);
        return FALLO;
    }

    if (SB.cantInodosLibres <= 0) // Si no quedan inodos libres, invocamos une error
    {
        fprintf(stderr, RED "Error en reservar_inodo. No hay ningun inodo libre." RESET);
        return FALLO;
    }

    int posInodoReservado = SB.posPrimerInodoLibre; // Guardamos la pos. del primer Inodo libre

    // Inicializamos el inodo reservado y sus valores iniciales
    struct inodo inodo;
    inodo.tipo = tipo;
    inodo.permisos = permisos;
    inodo.nlinks = 1;
    inodo.tamEnBytesLog = 0;
    inodo.atime = time(NULL);
    inodo.mtime = time(NULL);
    inodo.ctime = time(NULL);
    inodo.numBloquesOcupados = 0;

    for (int idx = 0; idx < 12; idx++)
    {
        for (int i = 0; i < 3; i++)
        {
            inodo.punterosIndirectos[i] = 0;
        }
        inodo.punterosDirectos[idx] = 0;
    }

    if (escribir_inodo(posInodoReservado, &inodo) == FALLO) // Escribimos el inodo que acabamos de reservar e inicializar
    {
        fprintf(stderr, RED "Error en reservar_inodo al escribir el inodo." RESET);
        return FALLO;
    }

    SB.cantInodosLibres--;    // Decrementamos la cantidad de Inodos Libres que hay en el superbloque
    SB.posPrimerInodoLibre++; // Hacemos que la pos. del primer inodo libre sea la posición siguiente

    if (bwrite(posSB, &SB) == FALLO)
    { // Sobreescribimos los cambios realizados en el superbloque
        fprintf(stderr, RED "Error en reservar_inodo al escribir en el superbloque." RESET);
        return FALLO;
    }
    return posInodoReservado; // La posición es relativa al Array de Inodos, no a la posición absoluta de todos los bloques
}

// NIVEL 4

/* Obtenemos el rango de punteros en el que se situa el bloque lógico que buscamos, y la dirección almacenada en el inodo
 * inodo: inodo del cual queremos obtener su direccion almacenada
 * nblogico: bloque lógico del cual hay que obtener su rango de punteros
 * ptr: para obtener la dirección almacenada en el inodo
 */
int obtener_nRangoBL(struct inodo *inodo, unsigned int nblogico, unsigned int *ptr)
{

    if (nblogico < DIRECTOS)
    { // <12
        *ptr = inodo->punterosDirectos[nblogico];
        return 0;
    }
    else if (nblogico < INDIRECTOS0)
    { // <268
        *ptr = inodo->punterosIndirectos[0];
        return 1;
    }
    else if (nblogico < INDIRECTOS1)
    { // <65.804
        *ptr = inodo->punterosIndirectos[1];
        return 2;
    }
    else if (nblogico < INDIRECTOS2)
    { // <16.843.020
        *ptr = inodo->punterosIndirectos[2];
        return 3;
    }
    else
    {
        *ptr = 0;
        fprintf(stderr, RED "Bloque lógico fuera de rango" RESET);
        return FALLO;
    }
}

/* Devolvemos el índice de los bloques de punteros
 * nblogico: nº del bloque lógico a localizar
 * nivel_punteros: simplemente el nivel de los punteros
 */
int obtener_indice(unsigned int nblogico, int nivel_punteros)
{
    if (nblogico < DIRECTOS)
    { // ej. nblogico=8
        return nblogico;
    }
    else if (nblogico < INDIRECTOS0)
    { // ej. nblogico=204
        return (nblogico - DIRECTOS);
    }
    else if (nblogico < INDIRECTOS1)
    { // ej. nblogico=30.004
        if (nivel_punteros == 2)
        {
            return ((nblogico - INDIRECTOS0) / NPUNTEROS);
        }
        else if (nivel_punteros == 1)
        {
            return ((nblogico - INDIRECTOS0) % NPUNTEROS);
        }
    }
    else if (nblogico < INDIRECTOS2)
    { // ej. nblogico=400.004
        if (nivel_punteros == 3)
        {
            return ((nblogico - INDIRECTOS1) / (NPUNTEROS * NPUNTEROS));
        }
        else if (nivel_punteros == 2)
        {
            return (((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) / NPUNTEROS);
        }
        else if (nivel_punteros == 1)
        {
            return (((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) % NPUNTEROS);
        }
    }
    return FALLO; // Si no se ha encontrado, devolvemos FALLO
}

/* Se obtiene el nº de bloque físico correspondiente a un bloque lógico del inodo indicado
 * inodo: inodo del cual obtendremos el bloque lógico
 * nblogico: nº de bloque lógico del cual hay que obtener el número de bloque físico
 * reservar: para determinar si el bloque existe o no
 */
int traducir_bloque_inodo(struct inodo *inodo, unsigned int nblogico, unsigned char reservar)
{
    // Inicializamos las variables
    unsigned int ptr = 0, ptr_ant = 0;
    int nRangoBL = obtener_nRangoBL(inodo, nblogico, &ptr); // 0:D, 1:I0, 2:I1, 3:I2
    if (nRangoBL == FALLO)
    {
        fprintf(stderr, RED "Error al obtener el rango en traducir_bloque_inodo." RESET);
        return FALLO;
    }
    int nivel_punteros = nRangoBL; // el nivel_punteros +alto es el que cuelga directamente del inodo
    unsigned int buffer[NPUNTEROS];
    int indice = 0;

    // iterar para cada nivel de punteros indirectos
    while (nivel_punteros > 0)
    {
        if (ptr == 0) // no cuelgan bloques de punteros
        {
            if (reservar == 0) // bloque inexistente -> no imprimir error por pantalla!!!
            {
                return FALLO;
            }
            else // reservar bloques de punteros y crear enlaces desde el inodo hasta el bloque de datos
            {
                ptr = reservar_bloque(); // de punteros
                if (ptr == FALLO)
                {
                    fprintf(stderr, RED "Error al intentar reservar el bloque en traducir_bloque_inodo." RESET);
                    return FALLO;
                }

                inodo->numBloquesOcupados++;
                inodo->ctime = time(NULL); // fecha actual

                if (nivel_punteros == nRangoBL) // el bloque cuelga directamente del inodo
                {
                    // printf(GRAY "[traducir_bloque_inodo()→ inodo.punterosIndirectos[%d] = %d (reservado BF %d para punteros_nivel%d)]\n" RESET, (nivel_punteros - 1), ptr, ptr, nivel_punteros);
                    inodo->punterosIndirectos[nRangoBL - 1] = ptr;
                }
                else // el bloque cuelga de otro bloque de punteros
                {
                    buffer[indice] = ptr;
                    if (bwrite(ptr_ant, buffer) == FALLO) // salvamos en el dispositivo el buffer de punteros modificado
                    {
                        fprintf(stderr, RED "Error al intentar escribir en el buffer de punteros modificado en traducir_bloque_inodo." RESET);
                        return FALLO;
                    }

                    // printf(GRAY "[traducir_bloque_inodo()→ punteros_nivel%d [%d] = %d (reservado BF %d para punteros_nivel%d)]\n" RESET, (nivel_punteros + 1), indice, ptr, ptr, nivel_punteros);
                }

                memset(buffer, 0, BLOCKSIZE); // ponemos a 0 todos los punteros del buffer
            }
        }
        else // revisar aca pouede que no sea else
        {
            if (bread(ptr, buffer) == FALLO) // leemos del dispositivo el bloque de punteros ya existente
            {
                fprintf(stderr, RED "Error al intentar leer el dispositivo del bloque de punteros en traducir_bloque_inodo." RESET);
                return FALLO;
            }
        }

        indice = obtener_indice(nblogico, nivel_punteros);
        if (indice == FALLO)
        {
            fprintf(stderr, RED "Error al obtener el índice en traducir_bloque_inodo." RESET);
            return FALLO;
        }
        ptr_ant = ptr;        // guardamos el puntero actual
        ptr = buffer[indice]; // y lo desplazamos al siguiente nivel

        nivel_punteros--;
    } // al salir de este bucle ya estamos al nivel de datos

    if (ptr == 0) // no existe bloque de datos
    {
        if (reservar == 0) // error lectura no existe bloque -> no imprimir error por pantalla!!!
        {
            return FALLO;
        }
        else
        {
            ptr = reservar_bloque(); // de datos

            if (ptr == FALLO)
            {
                fprintf(stderr, RED "Error al reservar el bloque en traducir_bloque_inodo." RESET);
                return FALLO;
            }
            inodo->numBloquesOcupados++;
            inodo->ctime = time(NULL);

            if (nRangoBL == 0) // si era un puntero Directo
            {
                // printf(GRAY "traducir_bloque_inodo()→ inodo.punterosDirectos[%d] = %d (reservado BF %d para BL %d)]\n" RESET, nblogico, ptr, ptr, nblogico);
                inodo->punterosDirectos[nblogico] = ptr; // asignamos la direción del bl. de datos en el inodo
            }
            else
            {
                // printf(GRAY "[traducir_bloque_inodo()→ punteros_nivel%d [%d] = %d (reservado BF %d para BL %d)]\n" RESET, (nivel_punteros + 1), indice, ptr, ptr, nblogico);

                buffer[indice] = ptr; // asignamos la dirección del bloque de datos en el buffer

                if (bwrite(ptr_ant, buffer) == FALLO) // salvamos en el dispositivo el buffer de punteros modificado
                {
                    fprintf(stderr, RED "Error al salvar en el dispositivo el buffer de punteros modificado en traducir_bloque_inodo" RESET);
                    return FALLO;
                }
            }
        }
    }
    return ptr; // nº de bloque físico correspondiente al bloque de datos lógico, nblogico
}

// NIVEL 6

/*
 *   Liberamos un inodo y pasa a ser el primero en la lista de inodos libres
 *   ninodo: nº de inodo a liberar
 */
int liberar_inodo(unsigned int ninodo)
{
    struct inodo inodo;
    if (leer_inodo(ninodo, &inodo) == FALLO)
    { // Leemos el inodo indicado por parámetro
        fprintf(stderr, RED "Error al leer el inodo en liberar_inodo." RESET);
        return FALLO;
    }

    int bloq_liber = liberar_bloques_inodo(0, &inodo); // Liberamos todos los bloques del inodo (primerBL = 0 porque los liberamos todos [empezamos desde el primer bloque])
    if (bloq_liber == FALLO)
    {
        fprintf(stderr, RED "Error al liberar los bloques en liberar_inodo." RESET);
        return FALLO;
    }

    inodo.numBloquesOcupados -= bloq_liber; // Restamos los bloques liberados del inodo. Debería ser 0 siempre.

    inodo.tamEnBytesLog = 0;
    inodo.tipo = 'l';

    // Actualizamos la lista enlazada de inodos libres
    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO)
    { // Leemos el superbloque
        fprintf(stderr, RED "Error al leer el superbloque en liberar_inodo." RESET);
        return FALLO;
    }

    inodo.punterosDirectos[0] = SB.posPrimerInodoLibre; // Apuntamos al SB.posPrimerInodoLibre anterior
    SB.posPrimerInodoLibre = ninodo;

    SB.cantInodosLibres++;

    if (bwrite(posSB, &SB) == FALLO)
    { // Escribimos el superbloque
        fprintf(stderr, RED "Error al escribir el superbloque en liberar_inodo." RESET);
        return FALLO;
    }

    inodo.ctime = time(NULL); // Actualizamos el ctime
    if (escribir_inodo(ninodo, &inodo) == FALLO)
    { // Escribimos el inodo liberado
        fprintf(stderr, RED "Error al escribir el inodo en liberar_inodo." RESET);
        return FALLO;
    }

    return ninodo; // Devolvemos el número de inodo liberado
}

/* Libera todos los bloques ocupados a partir del bloque lógico indicado por primerBL
 * primerBL: indica a partir de qué bloque hay que empezar a liberar (incluido)
 * inodo: el inodo del cual vamos a borrar sus bloques
 */
int liberar_bloques_inodo(unsigned int primerBL, struct inodo *inodo)
{
    // libera los bloques de datos e índices iterando desde el primer bloque lógico a liberar hasta el último
    // por tanto explora las ramificaciones de punteros desde las hojas hacia la raíz en el inodo

    int nivel_punteros = 0, indice = 0;
    unsigned int ptr = 0;
    unsigned int nBL = 0, ultimoBL = 0;
    int nRangoBL = 0;
    unsigned int bloques_punteros[3][NPUNTEROS]; // array de bloques de punteros
    unsigned int bufAux_punteros[NPUNTEROS];     // para llenar de 0s y comparar
    int ptr_nivel[3];                            // punteros a bloques de punteros de cada nivel
    int indices[3];                              // indices de cada nivel
    int liberados = 0;                           // nº de bloques liberados

    if (inodo->tamEnBytesLog == 0)
    { // el fichero está vacío -> devolvemos 0
        return liberados;
    }

    // obtenemos el último bloque lógico del inodo
    if (inodo->tamEnBytesLog % BLOCKSIZE == 0)
    {
        ultimoBL = inodo->tamEnBytesLog / BLOCKSIZE - 1;
    }
    else
    {
        ultimoBL = inodo->tamEnBytesLog / BLOCKSIZE;
    }

    memset(bufAux_punteros, 0, BLOCKSIZE);
    // printf(GRAY "[liberar_bloques_inodo()→ primer BL: %d, último BL: %d]\n", primerBL, ultimoBL);
    for (nBL = primerBL; nBL <= ultimoBL; nBL++)
    { // recorrido BLs

        nRangoBL = obtener_nRangoBL(inodo, nBL, &ptr); // 0:D, 1:I0, 2:I1, 3:I2

        if (nRangoBL == FALLO)
        {
            fprintf(stderr, RED "Error al obtener el nRangoBL en liberar_bloques_inodo." RESET);
            return FALLO;
        }

        nivel_punteros = nRangoBL; // el nivel_punteros +alto cuelga del inodo

        while (ptr > 0 && nivel_punteros > 0)
        { // cuelgan bloques de punteros
            indice = obtener_indice(nBL, nivel_punteros);

            if (indice == FALLO)
            {
                fprintf(stderr, RED "Error al obtener el indice en liberar_bloques_inodo." RESET);
                return FALLO;
            }

            if (indice == 0 || nBL == primerBL)
            {
                // solo hay que leer del dispositivo si no está ya cargado previamente en un buffer
                bread(ptr, bloques_punteros[nivel_punteros - 1]);
            }

            ptr_nivel[nivel_punteros - 1] = ptr;
            indices[nivel_punteros - 1] = indice;
            ptr = bloques_punteros[nivel_punteros - 1][indice];
            nivel_punteros--;
        }

        if (ptr > 0)
        { // si existe bloque de datos
            if (liberar_bloque(ptr) == FALLO)
            { // de datos
                fprintf(stderr, RED "Error al liberar el bloque apuntado por ptr 1 en liberar_bloques_inodo." RESET);
                return FALLO;
            }
            liberados++;

            // printf(GRAY "[liberar_bloques_inodo()→ liberado BF %d de datos para BL: %d]\n", ptr, nBL);
            if (nRangoBL == 0)
            { // es un puntero Directo
                inodo->punterosDirectos[nBL] = 0;
            }
            else
            { // es un puntero Indirecto
                nivel_punteros = 1;
                while (nivel_punteros <= nRangoBL)
                {
                    indice = indices[nivel_punteros - 1];
                    bloques_punteros[nivel_punteros - 1][indice] = 0;
                    ptr = ptr_nivel[nivel_punteros - 1];

                    if (memcmp(bloques_punteros[nivel_punteros - 1], bufAux_punteros, BLOCKSIZE) == EXITO)
                    {

                        // No cuelgan más bloques ocupados, hay que liberar el bloque de punteros
                        if (liberar_bloque(ptr) == FALLO)
                        { // Liberamos el bloque de punteros
                            fprintf(stderr, RED "Error al liberar el bloque apuntado por ptr 2 en liberar_bloques_inodo." RESET);
                            return FALLO;
                        }
                        liberados++;

                        // printf(GRAY "[liberar_bloques_inodo()→ liberado BF %d de punteros_nivel%d correspondiente al BL: %d]\n", ptr, nivel_punteros, nBL);

                        if (nivel_punteros == nRangoBL)
                        {
                            inodo->punterosIndirectos[nRangoBL - 1] = 0;
                        }

                        nivel_punteros++;
                    }
                    else
                    { // escribimos en el dispositivo el bloque de punteros modificado
                        if (bwrite(ptr, bloques_punteros[nivel_punteros - 1]) == FALLO)
                        {
                            fprintf(stderr, RED "Error al escribir el bloque en liberar_bloques_inodo." RESET);
                            return FALLO;
                        }
                        // hemos de salir del bucle ya que no será necesario liberar los bloques de niveles
                        // superiores de los que cuelga
                        nivel_punteros = nRangoBL + 1;
                    }
                }
            }
        }
    }
    // printf(GRAY "liberados: %d\n", liberados);
    return liberados;
}
