#include "directorios.h"

/* Programa que muestra información que consideremos relevante.
 * args: cantidad de argumentos
 * argv: lista de argumentos: [1] = nombre del dispositivo;
 */
int main(int args, char **argv)
{
    if (args != 2)
    {
        fprintf(stderr, "Uso: %s <nombre_dispositivo>\n", argv[0]);
        return FALLO;
    }

    const char *nombre_dispositivo = argv[1];

    // Montar el dispositivo virtual
    if (bmount(nombre_dispositivo) == FALLO)
    {
        fprintf(stderr, RED "Error al montar el dispositivo virtual.\n" RESET);
        return FALLO;
    }

    /*  COMENTADO PARA DEBUGGING DE LIBERAR_BLOQUES_INODO
        SB->posPrimerBloqueMB += 50;
        SB->posUltimoBloqueMB += 500;
        SB->posPrimerBloqueAI += 5000;
        SB->posUltimoBloqueAI += 10;
        SB->posPrimerBloqueDatos += 5;
        SB->posUltimoBloqueDatos += 80;
        SB->posInodoRaiz += 4;
        SB->posPrimerInodoLibre += 2;
        SB->cantBloquesLibres += 9;
        SB->cantInodosLibres += 1;
        SB->totBloques += 2;
        SB->totInodos += 500;*/

    // Modificar los datos del superbloque mmap
    printf("DATOS DEL SUPERBLOQUE\n");
    printf("posPrimerBloqueMB +50\n");
    printf("posUltimoBloqueMB +500\n");
    printf("posPrimerBloqueAI +5000\n");
    printf("posUltimoBloqueAI +10 \n");
    printf("posPrimerBloqueDatos +5\n");
    printf("posUltimoBloqueDatos +80\n");
    printf("posInodoRaiz +4\n");
    printf("posPrimerInodoLibre +2 \n");
    printf("cantBloquesLibres +9\n");
    printf("cantInodosLibres +1 \n");
    printf("totBloques +2\n");
    printf("totInodos +500 \n");
    printf("\n");

    // Desmontar el dispositivo virtual
    if (bumount() == FALLO)
    {
        fprintf(stderr, RED "Error al desmontar el dispositivo virtual.\n" RESET);
        return FALLO;
    }

    return EXITO;
}