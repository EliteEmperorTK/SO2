#include "ficheros_basico.h"

// Calcula el tamaño en bloques necesario para el mapa de bits.
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

// Calcula el tamaño en bloques del array de inodos.
int tamAI(unsigned int ninodos)
{ // Se dice algo sobre que el mi_mkfs.c pasará este dato que será ninodos=nbloques/4
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

// Inicializa los datos del superbloque.
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
    // Actualización del primer inodo libre reservar_inodo('d',7), posPrimerInododLibre++;reservar_indodo-->++, liberar_indod--;
    SB.cantBloquesLibres = nbloques;
    // reservar bloque-->--,liberar bloque-->++;
    SB.cantInodosLibres = ninodos;
    // reservar inodo-->--, liberar inodo-->++;
    SB.totBloques = nbloques;
    SB.totInodos = ninodos; // ninodos=nbloques/4;

    // Escribir la estructura en el bloque posSB
    if (bwrite(posSB, &SB) == FALLO)
    {
        fprintf(stderr, "Error al escribir el superbloque en el disco.\n");
        return FALLO;
    }

    return EXITO;
}

int block_size(int bytes)
{
    return (bytes + BLOCKSIZE - 1) / BLOCKSIZE;
}

// Inicializa el mapa de bits poniendo a 1 los bits que representan los metadatos.
int initMB()
{ // Mapa de bits
    struct superbloque SB;

    // Leer el superbloque para obtener información relevante
    if (bread(posSB, &SB) < 0)
    {
        fprintf(stderr, "Error al leer el superbloque en el disco.\n");
        return FALLO;
    }

    // Calcular el tamaño de los metadatos en bloques
    const int metadata_block_size = tamMB(SB.totBloques) + tamAI(SB.totInodos) + tamSB;

    // Calcular el tamaño del mapa de bits necesario en bytes
    const int metadata_bytes = metadata_block_size / 8;
    const int metadata_extra_bits = metadata_block_size % 8;

    // Calcular el tamaño del bloque del mapa de bits y el tamaño total en bytes
    const int bitmap_block_size = block_size(metadata_bytes + (metadata_extra_bits > 0));
    const int bitmap_byte_size = bitmap_block_size * BLOCKSIZE;

    // Crear un buffer para almacenar el mapa de bits en memoria
    unsigned char bitmap[bitmap_byte_size];

    // Llenar los bytes correspondientes a los metadatos con 1s
    memset(bitmap, 255, metadata_bytes);

    // Llenar el último byte parcial con 1s y 0s según los bits restantes de los metadatos
    memset(bitmap + metadata_bytes + 1, 0, bitmap_byte_size - metadata_bytes);
    bitmap[metadata_bytes] = 255 << (8 - metadata_extra_bits);

    // Escribir el mapa de bits en el disco
    for (int i = 0; i < bitmap_block_size; ++i)
    {
        if (bwrite(SB.posPrimerBloqueMB + i, &bitmap[i * BLOCKSIZE]) < 0)
        {
            return FALLO;
        }
    }

    // Actualizar la cantidad de bloques libres en el superbloque
    SB.cantBloquesLibres -= metadata_block_size;

    // Escribir el superbloque actualizado en el disco
    return bwrite(posSB, &SB);

    /*
        struct superbloque SB;
        if (bread(posSB, &SB) == FALLO)
        {
            fprintf(stderr, "Error al leer el superbloque en el disco.\n");
            return FALLO;
        }

        // posicion del bloque de mapa de bits
        int posBloqueMB = SB.posPrimerBloqueMB;

        // numero de bloques que ocupan los metadatos
        int nBloquesMD = tamSB + tamMB(SB.totBloques) + tamAI(SB.totInodos);

        int bytesTotales = nBloquesMD / 8;
        int bitsSobrantes = nBloquesMD % 8;

        //int nBloquesOcupan = nBloquesMD /8 / BLOCKSIZE;/////////////////
        int nBloquesOcupan = (bytesTotales + bitsSobrantes) * BLOCKSIZE;

        unsigned char bufferMB[BLOCKSIZE];

        //// Marcar como ocupados los bloques que ocupan los metadatos
        while (nBloquesOcupan > 0)
        {
            memset(bufferMB, 255, BLOCKSIZE);
            nBloquesOcupan--;

            // Escribir el bloque en el dispositivo
            if (bwrite(posBloqueMB, bufferMB) == FALLO)
            {
                fprintf(stderr, "Error al escribir en el bloque %d.\n", posBloqueMB);
                return FALLO;
            }
            posBloqueMB++;
        }

        // Marcar los bits adicionales en el último bloque
        memset(bufferMB, 0, sizeof(bufferMB));

        int i;
        for (i = 0; i <= (nBloquesMD / 8) - 1; i++)
        {
            bufferMB[i] = 255;
        }
        int bitsExtra = nBloquesMD % 8;
        switch (bitsExtra)
        {
        case 1:
            bufferMB[nBloquesMD / 8] = 128;
            break;
        case 2:
            bufferMB[nBloquesMD / 8] = 192;
            break;
        case 3:
            bufferMB[nBloquesMD / 8] = 224;
            break;
        case 4:
            bufferMB[nBloquesMD / 8] = 240;
            break;
        case 5:
            bufferMB[nBloquesMD / 8] = 248;
            break;
        case 6:
            bufferMB[nBloquesMD / 8] = 252;
            break;
        case 7:
            bufferMB[nBloquesMD / 8] = 254;
            break;

        default:
            break;
        }
        for (int j = i + 1; j <= BLOCKSIZE - 1; j++)
        {
            bufferMB[j] = 0;
        }

            // Escribir el último bloque del mapa de bits
        if (bwrite(posBloqueMB, bufferMB) == FALLO)
        {
            fprintf(stderr, "Error al escribir el último bloque del mapa de bits en el disco.\n");
            return FALLO;
        }

        // Actualizamos los bloques libres en el MB
        SB.cantBloquesLibres -= nBloquesMD;

        // Actualizar el superbloque
        if (bwrite(posSB, &SB) == FALLO)
        {
            fprintf(stderr, "Error al actualizar el superbloque en el disco.\n");
            return FALLO;
        }

        return EXITO;
        */
}

int initAI()
{ // Array Inodos
    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO)
    {
        fprintf(stderr, "Error al leer el superbloque en el disco.\n");
        return FALLO;
    }

    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    unsigned int contInodos = SB.posPrimerInodoLibre + 1;

    for (int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++)
    { // Para cada bloque del array de inodos

        // leer el bloque de inodos en el dispositivo virtual
        if (bread(i, inodos) == FALLO)
        {
            fprintf(stderr, "Error al leer el bloque de inodos en el disco.\n");
            return FALLO;
        }

        // Inicializar cada inodo del bloque
        for (int j = 0; j < BLOCKSIZE / INODOSIZE; j++)
        {                         // Para cada inodo del bloque
            inodos[j].tipo = 'l'; // libre

            if (contInodos < SB.totInodos)
            {                                               // Si no hemos llegado al último inodo del array de inodos
                inodos[j].punterosDirectos[0] = contInodos; // Enlazamos con el siguiente inodo libre
                contInodos++;
            }
            else
            { // Hemos llegado al último inodo
                inodos[j].punterosDirectos[0] = UINT_MAX;
                break;
            }
        }
        // Escribir el bloque de inodos en el dispositivo virtual
        if (bwrite(i, inodos) == FALLO)
        {
            fprintf(stderr, "Error al leer el superbloque en el disco.\n");
            return FALLO;
        }
    }
    return EXITO;
}

int escribir_bit(unsigned int nbloque, unsigned int bit)
{
    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO)
    {
        fprintf(stderr, "Error al leer el superbloque.\n");
        return FALLO;
    }

    int posbyte = nbloque / 8;
    int posbit = nbloque % 8;
    int nbloqueMB = posbyte / BLOCKSIZE;
    int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;
    unsigned char bufferMB[BLOCKSIZE];
    if (bread(nbloqueabs, bufferMB) == FALLO)
    {
        fprintf(stderr, "Error al leer el bloque de datos en el disco.\n");
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
        fprintf(stderr, "Error al escribir el bit");
        return FALLO;
    }
    return EXITO;
}

char leer_bit(unsigned int nbloque)
{
    // Leer el superbloque para obtener la localización del MB.
    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO)
    {
        fprintf(stderr, "Error al leer el superbloque.\n");
        return FALLO;
    }

    // Calculamos qué byte, posbyte, contiene el bit que representa el bloque nbloque en el MB, y luego la posición del bit dentro de ese byte, posbit
    int posbyte = nbloque / 8;
    int posbit = nbloque % 8;

    // Hemos de determinar luego en qué bloque del MB, nbloqueMB, se halla ese byte :
    int nbloqueMB = posbyte / BLOCKSIZE;

    // para BufferMB
    posbyte = posbyte % BLOCKSIZE;

    unsigned char mascara = 128; // 10000000
    unsigned char bufferMB[BLOCKSIZE];

    // posición absoluta del dispositivo virtual se encuentra ese bloque, nbloqueabs, donde leer/escribir el bit:
    int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;

    if (bread(nbloqueabs, bufferMB) == FALLO)
    {
        fprintf(stderr, "Error al leer el bloque de datos en el disco.\n");
        return FALLO;
    }
    mascara >>= posbit;           // desplazamiento de bits a la derecha, los que indique posbit
    mascara &= bufferMB[posbyte]; // operador AND para bits
    mascara >>= (7 - posbit);     // desplazamiento de bits a la derecha para dejar el 0 o 1 en el extremo derecho y leerlo en decimal
    return mascara;
}

// si no hay error en esta funcion en que otra puede haber inits?
int reservar_bloque()
{
    struct superbloque SB;
    unsigned char bufferMB[BLOCKSIZE];
    int nbloqueMB;
    int posbyte;
    int posbit;
    int nbloque;

    // Leer el superbloque
    if (bread(posSB, &SB) == FALLO)
    {
        fprintf(stderr, "Error al leer el superbloque en el disco.\n");
        return FALLO;
    }

    // Comprobar si quedan bloques libres
    if (SB.cantBloquesLibres <= 0)
    {
        fprintf(stderr, "No quedan bloques libres en el disco.\n");
        return FALLO;
    }

    // Iterar sobre los bloques del mapa de bits
    for (nbloqueMB = 0; nbloqueMB < SB.totBloques; nbloqueMB++)
    {
        // Leer el bloque del mapa de bits
        if (bread(nbloqueMB + SB.posPrimerBloqueMB, bufferMB) == FALLO)
        {
            fprintf(stderr, "Error al leer el bloque %d del mapa de bits.\n", nbloqueMB + SB.posPrimerBloqueMB);
            return FALLO;
        }
        // printf("%d\n", nbloqueMB + SB.posPrimerBloqueMB); // para debuggear
        //   Comparar con el buffer auxiliar para encontrar el primer byte con algún bit a 0
        unsigned char bufferAux[BLOCKSIZE];
        memset(bufferAux, 255, BLOCKSIZE); // llenamos el buffer auxiliar con bits a 1
        if (memcmp(bufferMB, bufferAux, BLOCKSIZE) != 0)
        {
            // Encontrar el primer byte con algún bit a 0
            for (posbyte = 0; posbyte < BLOCKSIZE; posbyte++)
            {
                if (bufferMB[posbyte] != 255)
                {
                    break;
                }
            }

            // Encontrar el primer bit a 0 dentro de ese byte
            unsigned char mascara = 128; // 10000000
            posbit = 0;
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
                fprintf(stderr, "Error al escribir el bit en la posición %d.\n", nbloque);
                return FALLO;
            }

            // Actualizar la cantidad de bloques libres en el superbloque
            SB.cantBloquesLibres--;

            // Guardar el superbloque actualizado
            if (bwrite(posSB, &SB) == FALLO)
            {
                fprintf(stderr, "Error al guardar el superbloque actualizado en el disco.\n");
                return FALLO;
            }

            // Limpiar el bloque en la zona de datos
            unsigned char bufferDatos[BLOCKSIZE];
            memset(bufferDatos, 0, BLOCKSIZE);
            if (bwrite(nbloque, bufferDatos) == FALLO)
            {
                fprintf(stderr, "Error al limpiar el bloque %d en la zona de datos.\n", nbloque);
                return FALLO;
            }

            // Devolver el número de bloque reservado
            return nbloque;
        }
    }

    fprintf(stderr, "No se pudo encontrar un bloque libre en el mapa de bits.\n");
    return FALLO;
}

int liberar_bloque(unsigned int nbloque)
{
    struct superbloque SB;
    unsigned char bufferMB[BLOCKSIZE];

    // Leer el superbloque para obtener información relevante
    if (bread(posSB, &SB) == FALLO)
    {
        fprintf(stderr, "Error al leer el superbloque en el disco.\n");
        return FALLO;
    }

    // Calcular el número de bloque dentro del mapa de bits (MB) //revisar
    unsigned int nbloqueMB = nbloque / (BLOCKSIZE * 8);

    // Leer el bloque correspondiente del mapa de bits
    if (bread(nbloqueMB + SB.posPrimerBloqueMB, bufferMB) == FALLO)
    {
        fprintf(stderr, "Error al leer el bloque del mapa de bits en el disco.\n");
        return FALLO;
    }

    // Calcular la posición del byte dentro del bloque del mapa de bits //revisar
    unsigned int posbyte = (nbloque % (BLOCKSIZE * 8)) / 8;

    // Calcular la posición del bit dentro del byte //revisar
    unsigned int posbit = nbloque % 8;

    // Actualizar el bit correspondiente a 0 en el buffer del mapa de bits //revisar
    bufferMB[posbyte] &= ~(1 << (7 - posbit));

    // Escribir el bloque actualizado del mapa de bits en el dispositivo
    if (bwrite(nbloqueMB + SB.posPrimerBloqueMB, bufferMB) == FALLO)
    {
        fprintf(stderr, "Error al escribir el bloque del mapa de bits en el disco.\n");
        return FALLO;
    }

    // Incrementar la cantidad de bloques libres en el superbloque
    SB.cantBloquesLibres++;

    // Escribir el superbloque actualizado en el dispositivo
    if (bwrite(posSB, &SB) == FALLO)
    {
        fprintf(stderr, "Error al escribir el superbloque en el disco.\n");
        return FALLO;
    }

    // Devolver el número de bloque liberado
    return nbloque;
}

int escribir_inodo(unsigned int ninodo, struct inodo *inodo)
{
    struct superbloque SB;

    if (bread(posSB, &SB) == FALLO)
    {
        perror(RED "Error en escribir_inodo al leer el superbloque." RESET);
        return FALLO;
    }

    // Encontramos el bloque
    int nBloqueAI = (ninodo * INODOSIZE) / BLOCKSIZE;
    int nBloqueAbs = nBloqueAI + SB.posPrimerBloqueAI;

    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    if (bread(nBloqueAbs, inodos) == FALLO)
    {
        // Error al leer el bloque
        perror(RED "Error en escribir_inodo al leer el bloque." RESET);
        return FALLO;
    }

    int posInodo = ninodo % (BLOCKSIZE / INODOSIZE);
    inodos[posInodo] = *inodo;

    if (bwrite(nBloqueAbs, inodos) == FALLO)
    {
        // Error al escribir el bloque
        perror(RED "Error en escribir_inodo al escribir el bloque." RESET);
        return FALLO;
    }
    return EXITO;
}

int leer_inodo(unsigned int ninodo, struct inodo *inodo)
{
    struct superbloque SB;

    if (bread(posSB, &SB) == FALLO)
    {
        perror(RED "Error en leer_inodo al leer el superbloque." RESET);
        return FALLO;
    }

    // Encontramos el bloque
    int nBloqueAI = (ninodo * INODOSIZE) / BLOCKSIZE;
    int nBloqueAbs = nBloqueAI + SB.posPrimerBloqueAI;

    struct inodo inodos[BLOCKSIZE / INODOSIZE];

    if (bread(nBloqueAbs, inodos) == FALLO)
    {
        // Error al leer el bloque
        perror(RED "Error en escribir_inodo al leer el bloque." RESET);
        return FALLO;
    }

    int posInodo = ninodo % (BLOCKSIZE / INODOSIZE);
    *inodo = inodos[posInodo];
    return EXITO;
}

int reservar_inodo(unsigned char tipo, unsigned char permisos)
{
    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO)
    {
        perror(RED "Error en reservar_inodo al leer el superbloque." RESET);
        return FALLO;
    }

    if (SB.cantInodosLibres == 0)
    { // Si no quedan inodos libres, invocamos une error
        perror(RED "Error en reservar_inodo. No hay ningun inodo libre." RESET);
        return FALLO;
    }

    int posInodoReservado = SB.posPrimerInodoLibre; // Guardamos la pos. del primer Inodo libre
    SB.posPrimerInodoLibre++;                       // Hacemos que la pos. del primer inodo libre sea la posición siguiente

    // Inicializamos el inodo reservado y sus valores iniciales
    struct inodo inodos;
    inodos.tipo = tipo;
    inodos.permisos = permisos;
    inodos.nlinks = 1;
    inodos.tamEnBytesLog = 0;
    inodos.atime = time(NULL);
    inodos.mtime = time(NULL);
    inodos.ctime = time(NULL);
    inodos.numBloquesOcupados = 0;
    inodos.punterosDirectos[0] = 0;
    inodos.punterosIndirectos[0] = 0;

    if (escribir_inodo(posInodoReservado, &inodos) == FALLO)
    { // Escribimos el inodo que acabamos de reservar e inicializar
        perror(RED "Error en reservar_inodo al escribir el inodo." RESET);
        return FALLO;
    }

    SB.cantInodosLibres--; // Decrementamos la cantidad de Inodos Libres que hay en el superbloque

    if (bwrite(posSB, &SB) == FALLO)
    { // Sobreescribimos los cambios realizados en el superbloque
        perror(RED "Error en reservar_inodo al escribir en el superbloque." RESET);
        return FALLO;
    }
    return posInodoReservado; // La posición es relativa al Array de Inodos, no a la posición absoluta de todos los bloques
}

int obtener_nRangoBL(struct inodo *inodos, unsigned int nblogico, unsigned int *ptr)
{ // Devolvemos el nrangoBL

    if (nblogico < DIRECTOS)
    { // <12
        *ptr = inodos->punterosDirectos[nblogico];
        return 0;
    }
    else if (nblogico < INDIRECTOS0)
    { // <268
        *ptr = inodos->punterosIndirectos[0];
        return 1;
    }
    else if (nblogico < INDIRECTOS1)
    { // <65.804
        *ptr = inodos->punterosIndirectos[1];
        return 2;
    }
    else if (nblogico < INDIRECTOS2)
    { // <16.843.020
        *ptr = inodos->punterosIndirectos[2];
        return 3;
    }
    else
    {
        *ptr = 0;
        perror(RED "Bloque lógico fuera de rango" RESET);
        return FALLO;
    }
}

int obtener_indice(unsigned int nblogico, int nivel_punteros)
{ // Devolvemos el índice
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

int traducir_bloque_inodo(struct inodo *inodos, unsigned int nblogico, unsigned char reservar)
{
    // Inicializamos las variables
    unsigned int ptr = 0;
    unsigned int ptr_ant = 0;

    int nRangoBL = obtener_nRangoBL(inodos, nblogico, &ptr); // 0:D, 1:I0, 2:I1, 3:I2

    ptr = 0; // =000000? porque cuando sale de la funcion se asigna -1480000 

    if (nRangoBL == FALLO)
    {
        perror(RED "Error al obtener el rango en traducir_bloque_inodo." RESET);
        return FALLO;
    }
    int nivel_punteros = nRangoBL; // el nivel_punteros +alto es el que cuelga directamente del inodo

    int indice = 0;
    unsigned int buffer[NPUNTEROS];
    
    while (nivel_punteros > 0)
    { // iterar para cada nivel de punteros indirectos

        if (ptr == 0)
        { // no cuelgan bloques de punteros
            if (reservar == 0)
            { // bloque inexistente -> no imprimir error por pantalla!!!
                return FALLO;
            }
            else
            {                            // reservarS bloques de punteros y crear enlaces desde el  inodo hasta el bloque de datos
                ptr = reservar_bloque(); // de punteros
                if (ptr == FALLO)
                { // Sobreescribimos los cambios realizados en el superbloque
                    perror(RED "Error al intentar reservar el bloque en traducir_bloque_inodo." RESET);
                    return FALLO;
                }

                inodos->numBloquesOcupados++;
                inodos->ctime = time(NULL); // fecha actual
                
                
                if (nivel_punteros == nRangoBL)
                { // el bloque cuelga directamente del inodo
                    //printf(GRAY "[traducir_bloque_inodo()→ inodo.punterosIndirectos[%d] = %d (reservado BF %d para punteros_nivel%d)]\n" RESET, (nivel_punteros - 1), ptr, ptr, nivel_punteros);
                    inodos->punterosIndirectos[nRangoBL - 1] = ptr;
                }
                else
                { // el bloque cuelga de otro bloque de punteros
                    buffer[indice] = ptr;
                    if (bwrite(ptr_ant, buffer) == FALLO)
                    { // salvamos en el dispositivo el buffer de punteros modificado
                        perror(RED "Error al intentar escribir en el buffer de punteros modificado en traducir_bloque_inodo." RESET);
                        return FALLO;
                    }
                }
                memset(buffer, 0, BLOCKSIZE); // ponemos a 0 todos los punteros del buffer
            }
        }
        else
        {
            if (bread(ptr, buffer) == FALLO)
            { // leemos del dispositivo el bloque de punteros ya existente
                perror(RED "Error al intentar leer el dispositivo del bloque de punteros en traducir_bloque_inodo." RESET);
                return FALLO;
            }
        }
        
        indice = obtener_indice(nblogico, nivel_punteros);
        if (indice == FALLO)
        {
            perror(RED "Error al obtener el índice en traducir_bloque_inodo." RESET);
            return FALLO;
        }
        ptr_ant = ptr;        // guardamos el puntero actual
        ptr = buffer[indice]; // y lo desplazamos al siguiente nivel
        //siempre que sale de acá vale 0, esta bien?

        //printf(GRAY "[traducir_bloque_inodo()→ punteros_nivel%d [%d] = %d (reservado BF %d para punteros_nivel%d)]\n" RESET, nivel_punteros, indice, ptr_ant, ptr_ant, (nivel_punteros - 1));

        nivel_punteros--;
    } // al salir de este bucle ya estamos al nivel de datos

    if (ptr == 0)
    { // no existe bloque de datos
        if (reservar == 0)
        {
            return -1;
        }
        else
        {
            ptr = reservar_bloque();

            if (ptr == FALLO)
            {
                perror(RED "Error al reservar el bloque en traducir_bloque_inodo." RESET);
                return FALLO;
            }
            inodos->numBloquesOcupados++;
            inodos->ctime = time(NULL);
            
            if (nRangoBL == 0)
            { // si era un puntero Directo
                //printf(GRAY "[traducir_bloque_inodo()→ inodo.punterosDirectos[%d] = %d (reservado BF %d para BL %d)]\n" RESET, nblogico, ptr, ptr, nblogico);
                inodos->punterosDirectos[nblogico] = ptr; // asignamos la direción del bl. de datos en el inodo
            }
            else
            {
                //printf(GRAY "[traducir_bloque_inodo()→ punteros_nivel%d [%d] = %d (reservado BF %d para BL %d)]\n" RESET, (nivel_punteros + 1), indice, ptr, ptr, nblogico);

                buffer[indice] = ptr; // asignamos la dirección del bloque de datos en el buffer
                if (bwrite(ptr_ant, buffer) == FALLO)
                { // salvamos en el dispositivo el buffer de punteros modificado
                    perror(RED "Error al salvar en el dispositivo el buffer de punteros modificado en traducir_bloque_inodo" RESET);
                    return FALLO;
                }
            }
        }
    }
    // mi_write_f() se encargará de salvar los cambios del inodo en disco
    return ptr; // nº de bloque físico correspondiente al bloque de datos lógico, nblogico
}
