/* AUTORES:
 * Marc Nadal Sastre Gondar
 * Joaquín Esperon Solari
 * Martí Vich Gispert
 */

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

    // Leer el superbloque
    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO)
    {
        fprintf(stderr, RED "Error al leer el superbloque.\n" RESET);
        bumount();
        return FALLO;
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

    /////////////
    // NIVEL 2 //
    /////////////
    /*
        // Mostrar el tamaño de los struct
        printf("sizeof struct superbloque: %lu\n", sizeof(struct superbloque));
        printf("sizeof struct inodo: %lu\n", sizeof(struct inodo));
        printf("\n");

        // Recorrer la lista enlazada de inodos libres
        printf("RECORRIDO LISTA ENLAZADA DE INODOS LIBRES\n");

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
            {// Leemos cada inodo del bloque

                if (contInodos < SB.totInodos)
                {                                               // Si no hemos llegado al último inodo del array de inodos
                    printf("%d ",inodos[j].punterosDirectos[0]);
                    contInodos++;
                }
                else
                { // Hemos llegado al último inodo
                    printf("%d ", inodos[j].punterosDirectos[0]);
                    break;
                }
            }
        }*/

    /////////////
    // NIVEL 3 //
    /////////////
    /*
    printf("RESERVAMOS UN BLOQUE Y LUEGO LO LIBERAMOS\n"); ////////////RESERVAR BLOQUE O LIBERAR_BLOQUE NO FUNCIONA BIEN/////////
    // Reservamos el bloque
    int nBloque = reservar_bloque();
    if (nBloque == FALLO)
    {
        fprintf(stderr, "Error al reservar un bloque.\n");
        return FALLO;
    }
    // actualizamos el SB
    if (bread(posSB, &SB) == FALLO)
    {
        fprintf(stderr, "Error al leer el superbloque.\n");
        bumount();
        exit(FALLO);
    }
    printf("Se ha reservado el bloque físico nº %d que era el 1º libre indicado por el MB \n", nBloque);
    printf("SB.cantBloquesLibres = %d \n", SB.cantBloquesLibres);

    // Liberamos el bloque
    if (liberar_bloque(nBloque) == FALLO)
    {
        fprintf(stderr, "Error al liberar el bloque.\n");
        return FALLO;
    }
    // actualizamos el SB
    if (bread(posSB, &SB) == FALLO)
    {
        fprintf(stderr, "Error al leer el superbloque.\n");
        bumount();
        exit(FALLO);
    }
    printf("Liberamos ese bloque y después SB.cantBloquesLibres = %d \n", SB.cantBloquesLibres);

    printf("\nMAPA DE BITS CON BLOQUES DE METADATOS OCUPADOS\n");

    //  printf(GRAY "[leer_bit(%d)→ posbyte:%d, posbyte (ajustado): %d, posbit:%d, nbloqueMB:%d, nbloqueabs:%d]\n" RESET, posSB, posSB / 8, posSB / 8, posSB % 8, posSB / 8 / 1024, posSB / 8 / 1024);
    printf("posSB : %d → leer_bit(%d) = %hhu\n\n", posSB, posSB, leer_bit(posSB));

    // printf(GRAY "[leer_bit(%d)→ posbyte:%d, posbyte (ajustado): %d, posbit:%d, nbloqueMB:%d, nbloqueabs:%d]\n" RESET, SB.posPrimerBloqueMB, SB.posPrimerBloqueMB / 8, SB.posPrimerBloqueMB / 8, SB.posPrimerBloqueMB % 8, SB.posPrimerBloqueMB / 8 / 1024, SB.posPrimerBloqueMB / 8 / 1024);
    printf("SB.posPrimerBloqueMB : %d → leer_bit(%d) = %hhu\n\n", SB.posPrimerBloqueMB, SB.posPrimerBloqueMB, leer_bit(SB.posPrimerBloqueMB));

    // printf(GRAY "[leer_bit(%d)→ posbyte:%d, posbyte (ajustado): %d, posbit:%d, nbloqueMB:%d, nbloqueabs:%d]\n" RESET, SB.posUltimoBloqueMB, SB.posUltimoBloqueMB / 8, SB.posUltimoBloqueMB / 8, SB.posUltimoBloqueMB % 8, SB.posUltimoBloqueMB / 8 / 1024, SB.posUltimoBloqueMB / 8 / 1024);
    printf("SB.posUltimoBloqueMB : %d → leer_bit(%d) = %hhu\n\n", SB.posUltimoBloqueMB, SB.posUltimoBloqueMB, leer_bit(SB.posUltimoBloqueMB));

    // printf(GRAY "[leer_bit(%d)→ posbyte:%d, posbyte (ajustado): %d, posbit:%d, nbloqueMB:%d, nbloqueabs:%d]\n" RESET, SB.posPrimerBloqueAI, SB.posPrimerBloqueAI / 8, SB.posPrimerBloqueAI / 8, SB.posPrimerBloqueAI % 8, SB.posPrimerBloqueAI / 8 / 1024, SB.posPrimerBloqueAI / 8 / 1024);
    printf("SB.posPrimerBloqueAI : %d → leer_bit(%d) = %hhu\n\n", SB.posPrimerBloqueAI, SB.posPrimerBloqueAI, leer_bit(SB.posPrimerBloqueAI));

    // printf(GRAY "[leer_bit(%d)→ posbyte:%d, posbyte (ajustado): %d, posbit:%d, nbloqueMB:%d, nbloqueabs:%d]\n" RESET, SB.posUltimoBloqueAI, SB.posUltimoBloqueAI / 8, SB.posUltimoBloqueAI / 8, SB.posUltimoBloqueAI % 8, SB.posUltimoBloqueAI / 8 / 1024, SB.posUltimoBloqueAI / 8 / 1024);
    printf("SB.posUltimoBloqueAI : %d → leer_bit(%d) = %hhu\n\n", SB.posUltimoBloqueAI, SB.posUltimoBloqueAI, leer_bit(SB.posUltimoBloqueAI));

    // printf(GRAY "[leer_bit(%d)→ posbyte:%d, posbyte (ajustado): %d, posbit:%d, nbloqueMB:%d, nbloqueabs:%d]\n" RESET, SB.posPrimerBloqueDatos, SB.posPrimerBloqueDatos / 8, SB.posPrimerBloqueDatos / 8, SB.posPrimerBloqueDatos % 8, SB.posPrimerBloqueDatos / 8 / 1024, SB.posPrimerBloqueDatos / 8 / 1024);
    printf("SB.posPrimerBloqueDatos : %d → leer_bit(%d) = %hhu\n\n", SB.posPrimerBloqueDatos, SB.posPrimerBloqueDatos, leer_bit(SB.posPrimerBloqueDatos));

    // printf(GRAY "[leer_bit(%d)→ posbyte:%d, posbyte (ajustado): %d, posbit:%d, nbloqueMB:%d, nbloqueabs:%d]\n" RESET, SB.posUltimoBloqueDatos, SB.posUltimoBloqueDatos / 8, SB.posUltimoBloqueDatos / 8, SB.posUltimoBloqueDatos % 8, SB.posUltimoBloqueDatos / 8 / 1024, SB.posUltimoBloqueDatos / 8 / 1024);
    printf("SB.posUltimoBloqueDatos : %d → leer_bit(%d) = %hhu\n\n", SB.posUltimoBloqueDatos, SB.posUltimoBloqueDatos, leer_bit(SB.posUltimoBloqueDatos));

    printf("\nDATOS DEL DIRECTORIO RAIZ\n");
    struct tm *ts;
    char atime[80];
    char mtime[80];
    char ctime[80];

    struct inodo inodo;
    int ninodo = SB.posInodoRaiz;
    leer_inodo(ninodo, &inodo); // Leemos el inodo Raíz

    // Sacamos los tiempos
    ts = localtime(&inodo.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodo.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);

    printf("tipo: %c \n", inodo.tipo);
    printf("permisos %hhu \n", inodo.permisos);
    printf("atime: %s \nmtime: %s \nctime: %s\n", atime, mtime, ctime); // Imprimimos los tiempos
    printf("nlinks: %d \n", inodo.nlinks);
    printf("tamEnBytesLog: %d \n", inodo.tamEnBytesLog);
    printf("numBloquesOcupados: %d \n", inodo.numBloquesOcupados);
    */

    //////////////
    // NIVEL 4  //
    //////////////
    /*
    int posInodoReservado = reservar_inodo('f', 6);

    struct inodo inodos;
    if (leer_inodo(posInodoReservado, &inodos) == FALLO) //
    {
        fprintf(stderr, "Error al leer el inodo en leer_sf.c .\n");
        return FALLO;
    }

    printf("\nINODO 1. TRADUCCION DE LOS BLOQUES LOGICOS 8, 204, 30.004, 400.004 y 468.750\n");
    printf("\n");

    traducir_bloque_inodo(&inodos, 8, 1);
    printf("\n");

    traducir_bloque_inodo(&inodos, 204, 1);
    printf("\n");

    traducir_bloque_inodo(&inodos, 30004, 1);
    printf("\n");

    traducir_bloque_inodo(&inodos, 400004, 1);
    printf("\n");

    traducir_bloque_inodo(&inodos, 468750, 1);
    printf("\n");

    printf("\nDATOS DEL INODO RESERVADO 1\n");
    // Sacamos los tiempos
    struct tm *ts;
    char atime[80];
    char mtime[80];
    char ctime[80];

    ts = localtime(&inodos.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodos.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&inodos.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);

    if (leer_inodo(posInodoReservado, &inodos) == FALLO) //
    {
        fprintf(stderr, "Error al leer el inodo en leer_sf.c .\n");
        return FALLO;
    }

    printf("tipo: %c \n", inodos.tipo);
    printf("permisos %hhu \n", inodos.permisos);
    printf("atime: %s \nmtime: %s \nctime: %s\n", atime, mtime, ctime); // Imprimimos los tiempos
    printf("nlinks: %d \n", inodos.nlinks);
    printf("tamEnBytesLog: %d \n", inodos.tamEnBytesLog);
    printf("numBloquesOcupados: %d \n\n", inodos.numBloquesOcupados);
    printf("SB.posPrimerInodoLibre = %d \n", SB.posPrimerInodoLibre);
    */

    /////////////
    // NIVEL 7 //
    /////////////
    /*
    void mostrar_buscar_entrada(char *camino, char reservar)
    {
        unsigned int p_inodo_dir = 0;
        unsigned int p_inodo = 0;
        unsigned int p_entrada = 0;
        int posible_error;
        printf("\ncamino: %s, reservar: %d\n", camino, reservar);
        if ((posible_error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, reservar, 6)) < 0)
        {
            mostrar_error_buscar_entrada(posible_error);
        }
        printf("**********************************************************************\n");
    }

    // Mostrar creación directorios y errores
    mostrar_buscar_entrada("pruebas/", 1);           // ERROR_CAMINO_INCORRECTO
    mostrar_buscar_entrada("/pruebas/", 0);          // ERROR_NO_EXISTE_ENTRADA_CONSULTA
    mostrar_buscar_entrada("/pruebas/docs/", 1);     // ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO
    mostrar_buscar_entrada("/pruebas/", 1);          // creamos /pruebas/
    mostrar_buscar_entrada("/pruebas/docs/", 1);     // creamos /pruebas/docs/
    mostrar_buscar_entrada("/pruebas/docs/doc1", 1); // creamos /pruebas/docs/doc1
    mostrar_buscar_entrada("/pruebas/docs/doc1/doc11", 1);
    // ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO
    mostrar_buscar_entrada("/pruebas/", 1);          // ERROR_ENTRADA_YA_EXISTENTE
    mostrar_buscar_entrada("/pruebas/docs/doc1", 0); // consultamos /pruebas/docs/doc1
    mostrar_buscar_entrada("/pruebas/docs/doc1", 1); // ERROR_ENTRADA_YA_EXISTENTE
    mostrar_buscar_entrada("/pruebas/casos/", 1);    // creamos /pruebas/casos/
    mostrar_buscar_entrada("/pruebas/docs/doc2", 1); // creamos /pruebas/docs/doc2

    */

    // Desmontar el dispositivo virtual
    if (bumount() == FALLO)
    {
        fprintf(stderr, RED "Error al desmontar el dispositivo virtual.\n" RESET);
        return FALLO;
    }

    return EXITO;
}
