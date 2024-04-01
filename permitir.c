#include "ficheros.h"

int main(int args, char **argv){
    //Validación sintaxis
    if(args!=4){
        fprintf("Sintaxis introducida incorrecta, sintaxis correcta: ./permitir <nombre_dispositivo><ninodo><permisos>");
        return -1;
    }
    void nombre_dispositivo=argv[1];
    int ninodo=atoi(argv[2]);
    int permisos=atoi(argv[3]);
    //montar dispositivo
    if(bmount(nombre_dispositivo)==-1){
        fprintf("Error en permitir.c al ejecutar bmount()\n");
        return -1;
    }
    //extraer de los argumentos numero de inodo y permisos del mismo
    if(mi_chmod_f(ninodo, permisos)==-1){
        fprintf("Error en permitir.c al ejercutar mi_chmod_f()\n")
        return -1;
    }
    //desmontar dispositivo
    if(bumount()==-1){
        fprintf("Error en permitir.c al ejecutar bumount()\n");
        return -1;
    }

    return 0;
}