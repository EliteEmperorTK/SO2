/* AUTORES:
* Marc Nadal Sastre Gondar
* Joaquín Esperon Solari
* Martí Vich Gispert
*/

#include "ficheros.h"

/* Programa ficticio que llama a liberar_inodo() y mi_truncar_f()
* args: cantidad de argumentos
* argv: lista de argumentos: [1] = nombre del dispositivo; [2] = nº inodo que vamos a truncar; [3] = cantidad de bytes hasta la que vamos a truncar
*/
int main(int args, char **argv)
{
    // Validación sintaxis
    if (args != 4)
    {
        fprintf(stderr, RED "Sintaxis incorrecta, sintaxis correcta: truncar <nombre_dispositivo> <ninodo> <nbytes>\n" RESET);
        return FALLO;
    }

    char *nombre_dispositivo = argv[1];
    int ninodo = atoi(argv[2]);
    int nbytes = atoi(argv[3]);

    // Montar dispositivo
    if (bmount(nombre_dispositivo) == FALLO)
    {
        fprintf(stderr, RED "Error en truncar.c, al ejecutar bmount\n" RESET);
        return FALLO;
    }

    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO)
    {
        fprintf(stderr, RED "Error al leer el superbloque.\n" RESET);
        bumount();
        return FALLO;
    }

    int cantBlo = SB.cantBloquesLibres;
    printf("CantBloquesLibres:%d\n", cantBlo);

    // Comprobación mi_truncar_f
    if (nbytes == 0)
    {
        if (liberar_inodo(ninodo) == FALLO)
        {
            fprintf(stderr, RED "Error al liberar el inodo en truncar.c" RESET);
            return FALLO;
        }
    }
    else
    {
        if (mi_truncar_f(ninodo, nbytes) == FALLO)
        {
            fprintf(stderr, RED "Error al realizar el mi_truncar_f en truncar.c" RESET);
            return FALLO;
        }
    }

    struct STAT stats;
    if (mi_stat_f(ninodo, &stats) == FALLO)
    {
        fprintf(stderr, RED "Error al asignar los valores con mi_stat_f en truncar.c");
        return FALLO;
    }

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
    if (bumount() == FALLO)
    {
        fprintf(stderr, RED "Error en truncar.c, al ejecutar bumount\n" RESET);
        return FALLO;
    }
    return EXITO;
}
