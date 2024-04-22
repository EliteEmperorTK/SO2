#include "ficheros.h"

int main(int args, char **argv)
{
    // Validación de argumentos
    if (args != 3)
    {
        printf("Sintaxis introducida incorrecta, sintaxis correcta: './leer <nombre_dispositivo><ninodos>\n");
        return -1;
    }

    int tamBuffer = 1500; //
    if (bmount(argv[1]) == -1)
    { //
        printf("Error en leer.c al ejecutar bmount");
        return -1;
    }
    unsigned char buffer_texto[tamBuffer]; //
    int ninodo = atoi(argv[2]);            //
    int contbytesLeidos = 0;               //
    int offset = 0;                        //
    int leidos = 0;                        //
    struct STAT stat;                      //

    while ((leidos = mi_read_f(ninodo, buffer_texto, offset, tamBuffer)) > 0)
    {
        contbytesLeidos += leidos;          //
        write(1, buffer_texto, leidos);     //
        offset += tamBuffer;                //
        memset(buffer_texto, 0, tamBuffer); //

        // leidos = mi_read_f(ninodo, buffer_texto, offset, tamBuffer); //
    }

    char string[128];
    sprintf(string, "total_leidos: %d\n", contbytesLeidos); //
    write(2, string, strlen(string));                       //

    mi_stat_f(ninodo, &stat);
    sprintf(string, "tamEnBytesLog: %d\n", stat.tamEnBytesLog); //
    write(2, string, strlen(string));

    if (bumount(argv[1]) == -1)
    { //
        printf("Error en leer.c al ejecutar bumount");
        return -1;
    }
}