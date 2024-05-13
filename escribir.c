/* AUTORES:
* Marc Nadal Sastre Gondar
* Joaquín Esperon Solari
* Martí Vich Gispert
*/

#include "ficheros.h"

/* Escribe texto en uno o varios inodos
 * args: cantidad de argumentos
 * argv: lista de argumentos: [1] = nombre del dispositivo; [2] = fichero; [3] = cuantos inodos hay
 */
int main(int args, char **argv)
{
    // Validación de argumentos
    if (args != 4)
    {
        fprintf(stderr, RED "Sintaxis: escribir <nombre_dispositivo> <"
                            "$(cat fichero)"
                            "> <diferentes_inodos>\n Offsets: 9000, 209000, 30725000, 409605000, 480000000 \n Si diferentes_inodos=0 se reserva un solo inodo para todos los offsets" RESET);
        return FALLO;
    }
    // Inicialización argumentos
    char *nombre_dispositivo = argv[1];
    char *fichero = argv[2];
    int diferentes_inodos = atoi(argv[3]);

    int OFFSETS[5] = {9000, 209000, 30725000, 409605000, 480000000};

    int lon = strlen(fichero);
    char buffer[lon];

    // Copia de fichero pasado por parametro a buffer para poder manipularlo
    strcpy(buffer, fichero);

    // Montamos el dispositivo
    if (bmount(nombre_dispositivo) == FALLO)
    {
        fprintf(stderr, RED "Error en escribir.c al ejecutar bmount()" RESET);
        return FALLO;
    }
    struct STAT stats;
    int bytesEscritos = 0;
    int ninodo = 0;

    printf("longitud texto: %d\n", lon);

    if (diferentes_inodos == 0) // Tan solo un inodo
    {
        ninodo = reservar_inodo('f', 6);
        if (ninodo == FALLO)
        {
            fprintf(stderr, RED "Error al reservar inodo" RESET);
            return FALLO;
        }
        for (int idx = 0; idx <= 4; idx++)
        {
            // Imprimimos la información correspondiente
            printf("\nNº inodo reservado: %d\n", ninodo);

            printf("offset: %d\n", OFFSETS[idx]);

            bytesEscritos = mi_write_f(ninodo, fichero, OFFSETS[idx], lon);
            if (bytesEscritos == FALLO)
            {
                fprintf(stderr, RED "Error de escritura\n" RESET);
                return FALLO;
            }
            printf("Bytes escritos: %d\n", bytesEscritos);

            if (mi_stat_f(ninodo, &stats) == FALLO)
            {
                fprintf(stderr, RED "Error de lectura de STAT\n" RESET);
                return FALLO;
            }
            printf("stat.tamEnBytesLog= %d\n", stats.tamEnBytesLog);
            printf("stat.numBloquesOcupados= %d\n", stats.numBloquesOcupados);
        }
    }
    else // Multiples inodos
    {
        for (int idx = 0; idx <= 4; idx++)
        {
            // Imprimimos la información necesaria
            ninodo = reservar_inodo('f', 6);
            if (ninodo == FALLO)
            {
                fprintf(stderr, RED "Error al reservar inodo" RESET);
                return FALLO;
            }
            printf("\nNº inodo reservado: %d\n", ninodo);

            printf("offset: %d\n", OFFSETS[idx]);

            bytesEscritos = mi_write_f(ninodo, fichero, OFFSETS[idx], lon);
            if (bytesEscritos == FALLO)
            {
                fprintf(stderr, RED "Error de escritura\n" RESET);
                return FALLO;
            }
            printf("Bytes escritos: %d\n", bytesEscritos);

            if (mi_stat_f(ninodo, &stats) < 0)
            {
                fprintf(stderr, RED "Error de lectura de STAT\n" RESET);
                return FALLO;
            }
            printf("stat.tamEnBytesLog= %d\n", stats.tamEnBytesLog);
            printf("stat.numBloquesOcupados= %d\n", stats.numBloquesOcupados);
        }
    }

    if (bumount() == FALLO)
    {
        fprintf(stderr, RED "Error en escribir.c al llamar a bumount\n" RESET);
        return FALLO;
    }
}