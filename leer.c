#include "ficheros.h"

int main(int args, char **argv)
{
    // Validación de argumentos
    if (args != 3)
    {
        perror(RED "Sintaxis introducida incorrecta, sintaxis correcta: './leer <nombre_dispositivo><ninodos>\n" RESET);
        return FALLO;
    }
    if (bmount(argv[1]) == FALLO)
    {
        perror(RED "Error en leer.c al ejecutar bmount" RESET);
        return FALLO;
    }

    struct STAT stat;

    int tamBuffer = 1500;                  // tamaño buffer
    unsigned char buffer_texto[tamBuffer]; // creamos el bufer
    memset(buffer_texto, 0, tamBuffer);    // asignamos todo a 0

    int ninodo = atoi(argv[2]);

    int totalLeidos = 0;
    int offset = 0;
    int leidos = 0;

    leidos = mi_read_f(ninodo, buffer_texto, offset, tamBuffer); // leemos el inodo
    while (leidos > 0) //De mientras pueda leerse, lo hacemos
    {
        write(1, buffer_texto, leidos);
        totalLeidos += leidos; // añadimos los bytes leídos al contador total de bytes leídos
        offset += tamBuffer;
        memset(buffer_texto, 0, tamBuffer);                          // reiniciamos el valor del buffer
        leidos = mi_read_f(ninodo, buffer_texto, offset, tamBuffer); // volvemos a leer
    }

    char string[128];
    sprintf(string, "\ntotal_leidos: %d\n", totalLeidos); // imprimimos la cantidad de bytes leídos
    write(2, string, strlen(string));

    mi_stat_f(ninodo, &stat);
    sprintf(string, "tamEnBytesLog: %d\n", stat.tamEnBytesLog); // imprimimos la cantidad de bytes del fichero (debería ser igual a la cantidad de bytes leídos)
    write(2, string, strlen(string));

    if (bumount(argv[1]) == FALLO)
    {
        perror(RED "Error en leer.c al ejecutar bumount" RESET);
        return FALLO;
    }
    return EXITO;
}