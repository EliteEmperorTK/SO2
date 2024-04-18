#include "ficheros.h"

int main(int args, char **argv)
{
    // Validación de argumentos
    if (args != 4)
    {
        fprintf(stderr, RED "Sintaxis introducida incorrecta, sintaxis correcta: './escribir <nombre_dispositivo><"
                            "$(cat fichero)"
                            "><diferentes_inodos>" RESET);
        return -1;
    }
    // Inicialización argumentos
    char *nombre_dispositivo = argv[1];
    char *fichero = argv[2];
    int diferentes_inodos = atoi(argv[3]);
    int lon = strlen(fichero);
    int OFFSETS[5] = {9000, 209000, 30725000, 409605000, 480000000};
    char buffer[lon];
    // copia de fichero dado por parametro a buffer para manipulación
    strcpy(buffer, fichero);
    // montar dispositivo
    if (bmount(nombre_dispositivo) == -1)
    {
        fprintf(stderr, RED "Error en escribir.c al ejecutar bmount()" RESET);
        return -1;
    }
    struct STAT stats;
    int nInodo = reservar_inodo('f', 6);
    printf("longitud texto: %d\n", lon);
    int bytesEscritos = 0;
    for (int i = 0; i < 5; i++)
    {
        if (diferentes_inodos != 0)
        {
            nInodo = reservar_inodo('f', 6);
        }
        printf("\nNº inodo reservado: %d\n", nInodo);
        printf("offset: %d\n", OFFSETS[i]);
        bytesEscritos = mi_write_f(nInodo, fichero, OFFSETS[i], lon);
        memset(buffer, 0, lon);
        mi_stat_f(nInodo, &stats);
        printf("Bytes escritos: %d\n", bytesEscritos);
        printf("stat.tamEnBytesLog= %d\n", stats.tamEnBytesLog);
        printf("stat.numBloquesOcupados= %d\n", stats.numBloquesOcupados);
    }
    if (bumount() == FALLO)
    {
        fprintf(stderr, RED "Error en escribir.c --> %d: %s\n" RESET, errno, strerror(errno));
        return -1;
    }
}