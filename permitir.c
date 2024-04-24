#include "ficheros.h"

int main(int args, char **argv)
{
    // Validación sintaxis
    if (args != 4)
    {
        perror(RED "Sintaxis introducida incorrecta, sintaxis correcta: ./permitir <nombre_dispositivo><ninodo><permisos>" RESET);
        return FALLO;
    }
    char *nombre_dispositivo = argv[1];
    int ninodo = atoi(argv[2]);
    int permisos = atoi(argv[3]);

    
    if (bmount(nombre_dispositivo) == FALLO) // Montamos el dispositivo virtual
    {
        perror(RED "Error en permitir.c al ejecutar bmount()\n" RESET);
        return FALLO;
    }

    
    if (mi_chmod_f(ninodo, permisos) == FALLO) // Sacamos de los argumentos los permisos y el ninodo
    {
        perror( RED "Error en permitir.c al ejercutar mi_chmod_f()\n" RESET);
        return FALLO;
    }
    
    
    if (bumount() == FALLO) // Desmontamos el dispositivo virtual
    {
        perror(RED "Error en permitir.c al ejecutar bumount()\n" RESET);
        return FALLO;
    }

    return EXITO;
}