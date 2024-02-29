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

// Inicializa el mapa de bits poniendo a 1 los bits que representan los metadatos.
int initMB()
{ // Mapa de bits
    struct superbloque SB;
    // initSB();

    if (bread(posSB, &SB) == FALLO)
    {
        fprintf(stderr, "Error al leer el superbloque en el disco.\n");
        return FALLO;
    }

    int posBloqueMB = SB.posPrimerBloqueMB;
    int nBloquesMD = tamSB + tamMB(SB.totBloques) + tamAI(SB.totInodos);
    int nBloquesOcupan = nBloquesMD; // int nBloquesOcupan = nBloquesMD / 8 / BLOCKSIZE;

    unsigned char bufferMB[BLOCKSIZE];
    //// Marcar como ocupados los bloques que ocupan los metadatos
    while (nBloquesOcupan > 0)
    {
        memset(bufferMB, 255, sizeof(bufferMB));
        nBloquesOcupan--;

        SB.cantBloquesLibres--; // Actualizar la cantidad de bloques libres

        // Escribir el bloque en el dispositivo
        if (bwrite(posBloqueMB, bufferMB) == FALLO)
        {
            fprintf(stderr, "Error al escribir en el bloque %d.\n", posBloqueMB);
            return FALLO;
        }
        // bwrite(posBloqueMB, bufferMB);

        posBloqueMB++;
    }

    // Restaurar el bufferMB para el último bloque del mapa de bits
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
        bufferMB[i + 1] = 128;
        break;
    case 2:
        bufferMB[i + 1] = 192;
        break;
    case 3:
        bufferMB[i + 1] = 224;
        break;
    case 4:
        bufferMB[i + 1] = 240;
        break;
    case 5:
        bufferMB[i + 1] = 248;
        break;
    case 6:
        bufferMB[i + 1] = 252;
        break;
    case 7:
        bufferMB[i + 1] = 254;
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

    // Actualizar el superbloque
    if (bwrite(posSB, &SB) == FALLO)
    {
        fprintf(stderr, "Error al actualizar el superbloque en el disco.\n");
        return FALLO;
    }

    return EXITO;
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

/*
int escribir_bit(unsigned int nbloque, unsigned int bit)
{
}

char leer_bit(unsigned int nbloque)
{
}

int reservar_bloque()
{
}

int liberar_bloque(unsigned int nbloque)
{
}

int escribir_inodo(unsigned int ninodo, struct inodo *inodo)
{
}

int leer_inodo(unsigned int ninodo, struct inodo *inodo)
{
}

int reservar_inodo(unsigned char tipo, unsigned char permisos)
{
}*/
