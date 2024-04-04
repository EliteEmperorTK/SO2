#include "ficheros.h"

int main(int args, char** argv){
    //Validación de argumentos
    if(args!= 3){
        printf("Sintaxis introducida incorrecta, sintaxis correcta: './leer <nombre_dispositivo><ninodos>\n");
        return -1;
    }

    int tamBuffer=1500;
    if (bmount(argv[1])==-1){
        printf("Error en leer.c al ejecutar bmount");
        return -1;
    }
    unsigned char buffer [tamBuffer]; 
    int ninodo=atoi(argv[2]);
    int contbytesLeidos=0;
    int offset=0;
    int bytesLeidos=0;
    struct STAT stat;

    while((bytesLeidos=mi_read_f(ninodo,buffer, offset, tamBuffer))>0){
        contbytesLeidos+=bytesLeidos;
        write(1, buffer, bytesLeidos);
        offset += tamBuffer;
        memset(buffer, 0, tamBuffer);
        bytesLeidos=mi_read_f(ninodo,buffer, offset, tamBuffer);
    }
    mi_stat_f(ninodo, &stat);
    printf("total_leidos: %d\n",contbytesLeidos);
    printf("tamEnBytesLog: %d\n",stat.tamEnBytesLog);
    if(bumount()==-1){
        printf("Error en leer.c al ejecutar bumount");
        return -1;
    }
}