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

int initSB(unsigned int nbloques, unsigned int ninodos)
{ // Superbloques
}

int initMB()
{ // Mapa de bits
}

int initAI()
{ // Array Inodos
    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    unsigned int contInodos = SB.posPrimerInodoLibre + 1;
    int i, j;

    for (i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++) { // Para cada bloque del array de inodos

        // Inicializar cada inodo del bloque
        for (j = 0; j < BLOCKSIZE / INODOSIZE; j++) { // Para cada inodo del bloque
            inodos[j].tipo = 'l';

            if (contInodos < SB.totInodos) { // Si no hemos llegado al último inodo del array de inodos
                inodos[j].punterosDirectos[0] = contInodos; // Enlazamos con el siguiente inodo libre
                contInodos++;
            } else { // Hemos llegado al último inodo
                inodos[j].punterosDirectos[0] = UINT_MAX;
                break;
            }
        }
    }
    return 0;
}
