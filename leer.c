/* AUTORES:
* Marc Nadal Sastre Gondar
* Joaquín Esperon Solari
* Martí Vich Gispert
*/

#include "ficheros.h"

/* Lee el texto de un inodo
 * args: cantidad de argumentos
 * argv: lista de argumentos:  [1] = nombre del dispositivo; [2] = nº del inodo a leer
 */
int main(int args, char **argv)
{
    // Validación de argumentos
    if (args != 3)
    {
        fprintf(stderr, RED "Sintaxis: ./leer <nombre_dispositivo> <ninodo> \n" RESET);
        return FALLO;
    }

    // Inicialización argumentos
    char *nombre_dispositivo = argv[1];
    int ninodo = atoi(argv[2]);

    struct STAT stat;

    int tambuffer = 1500;                  // tamaño buffer
    unsigned char buffer_texto[tambuffer]; // creamos el bufer
    memset(buffer_texto, 0, tambuffer);    // asignamos todo a 0

    int totalLeidos = 0;
    int offset = 0;
    int leidos = 0;

    if (bmount(nombre_dispositivo) == FALLO)
    {
        return FALLO;
    }

    leidos = mi_read_f(ninodo, buffer_texto, offset, tambuffer); // leemos el inodo
    while (leidos > 0)                                           // De mientras pueda leerse, lo hacemos
    {
        write(1, buffer_texto, leidos);
        totalLeidos += leidos; // añadimos los bytes leídos al contador total de bytes leídos
        offset += tambuffer;
        memset(buffer_texto, 0, tambuffer);                          // reiniciamos el valor del buffer
        leidos = mi_read_f(ninodo, buffer_texto, offset, tambuffer); // volvemos a leer
    }

    char string[128];
    sprintf(string, "\ntotal_leidos: %d\n", totalLeidos); // imprimimos la cantidad de bytes leídos
    write(2, string, strlen(string));

    mi_stat_f(ninodo, &stat);
    sprintf(string, "tamEnBytesLog: %d\n", stat.tamEnBytesLog); // imprimimos la cantidad de bytes del fichero (debería ser igual a la cantidad de bytes leídos)
    write(2, string, strlen(string));

    if (bumount() == FALLO)
    {
        return FALLO;
    }
    return EXITO;
}