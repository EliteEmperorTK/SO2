#include "ficheros.h"

int main(int args, char **argv) // bien suponemos xd
{
    // Validación sintaxis
    if (args != 4)
    {
        perror(RED "Sintaxis incorrecta, sintaxis correcta: truncar <nombre_dispositivo> <ninodo> <nbytes>\n" RESET);
        return FALLO;
    }

    char *nombre_dispositivo = argv[1];
    int ninodo = atoi(argv[2]);
    int nbytes = atoi(argv[3]);

    // Montar dispositivo
    if (bmount(nombre_dispositivo) == FALLO)
    {
        perror(RED "Error en truncar.c, al ejecutar bmount\n" RESET);
        return FALLO;
    }

    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO)
    {
        perror(RED "Error al leer el superbloque.\n" RESET);
        bumount();
        exit(FALLO);
    }

    int cantBlo = SB.cantBloquesLibres;
    printf("CantBloquesLibres:%d\n", cantBlo);
    // Comprobación mi_truncar_f
    if (nbytes == 0)
    {
        liberar_inodo(ninodo);
    }
    else
    {
        mi_truncar_f(ninodo, nbytes);
    }

    struct STAT stats;
    mi_stat_f(ninodo, &stats);

    struct tm *ts;
    char atime[80];
    char mtime[80];
    char ctime[80];

    fprintf(stderr, "\nDATOS INODO %d\n", atoi(argv[2]));
    fprintf(stderr, CYAN "tipo: %c\n" RESET, stats.tipo);
    fprintf(stderr, "permisos: %d\n", stats.permisos);
    ts = localtime(&stats.atime);
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&stats.mtime);
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", ts);
    ts = localtime(&stats.ctime);
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", ts);
    printf("atime: %s \nmtime: %s \nctime: %s\n", atime, mtime, ctime);
    fprintf(stderr, "nlinks: %d\n", stats.nlinks);
    fprintf(stderr, CYAN "TamEnBytesLog: %d\n" RESET, stats.tamEnBytesLog);
    fprintf(stderr, CYAN "numBloquesOcupados: %d\n" RESET, stats.numBloquesOcupados);

    // Desmontar dispositivo
    if (bumount() == -1)
    {
        perror(RED "Error en truncar.c, al ejecutar bumount\n" RESET);
        return FALLO;
    }
    return EXITO;
}
