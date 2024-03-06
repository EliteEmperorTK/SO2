#include "ficheros_basico.h"

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Uso: %s <nombre_dispositivo>\n", argv[0]);
        exit(FALLO);
    }

    const char *nombre_dispositivo = argv[1];

    // Montar el dispositivo virtual
    if (bmount(nombre_dispositivo) == FALLO)
    {
        fprintf(stderr, "Error al montar el dispositivo virtual.\n");
        exit(FALLO);
    }

    // Leer el superbloque
    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO)
    {
        fprintf(stderr, "Error al leer el superbloque.\n");
        bumount();
        exit(FALLO);
    }

    // Mostrar los datos del superbloque
    printf("DATOS DEL SUPERBLOQUE\n");
    printf("posPrimerBloqueMB = %d\n", SB.posPrimerBloqueMB);
    printf("posUltimoBloqueMB = %d\n", SB.posUltimoBloqueMB);
    printf("posPrimerBloqueAI = %d\n", SB.posPrimerBloqueAI);
    printf("posUltimoBloqueAI = %d\n", SB.posUltimoBloqueAI);
    printf("posPrimerBloqueDatos = %d\n", SB.posPrimerBloqueDatos);
    printf("posUltimoBloqueDatos = %d\n", SB.posUltimoBloqueDatos);
    printf("posInodoRaiz = %d\n", SB.posInodoRaiz);
    printf("posPrimerInodoLibre = %d\n", SB.posPrimerInodoLibre);
    printf("cantBloquesLibres = %d\n", SB.cantBloquesLibres);
    printf("cantInodosLibres = %d\n", SB.cantInodosLibres);
    printf("totBloques = %d\n", SB.totBloques);
    printf("totInodos = %d\n", SB.totInodos);
    printf("\n");

    // Mostrar el tamaño de los struct
    printf("sizeof struct superbloque: %lu\n", sizeof(struct superbloque));
    printf("sizeof struct inodo: %lu\n", sizeof(struct inodo));
    printf("\n");

    // Recorrer la lista enlazada de inodos libres
    printf("RECORRIDO LISTA ENLAZADA DE INODOS LIBRES\n");
    int posInodoLibre = SB.posPrimerInodoLibre;

    while (posInodoLibre != -1)
    {
        printf("%d ", posInodoLibre);
        struct inodo i;

        if (bread(posInodoLibre, &i) == FALLO)
        {
            fprintf(stderr, "Error al leer el inodo libre.\n");
            bumount();
            exit(FALLO);
        }
        posInodoLibre = i.punterosDirectos[0];
    }
    printf("\n");

    // Desmontar el dispositivo virtual
    if (bumount() == FALLO)
    {
        fprintf(stderr, "Error al desmontar el dispositivo virtual.\n");
        exit(FALLO);
    }

    return EXITO;
}
