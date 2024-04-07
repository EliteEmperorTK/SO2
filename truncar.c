#include "ficheros.h"

int main(int args, char** argv){
    //Validación sintaxis
    if(args!=4){
        printf("Sintaxis incorrecta, sintaxis correcta: truncar <nombre_dispositivo> <ninodo> <nbytes>\n");
        return -1;
    }
    
    char* nombre_dispositivo=argv[1];
    int ninodo=atoi(argv[2]);
    int nbytes=atoi(argv[3]);
    //Montar dispositivo
    if(bmount(nombre_dispositivo)==-1){
        printf("Error en truncar.c, al ejecutar bmount\n");
        return -1;
    }
    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO)
    {
        fprintf(stderr, "Error al leer el superbloque.\n");
        bumount();
        exit(FALLO);
    }
    int cantBlo=SB.cantBloquesLibres;
    printf("CantBloquesLibres:%d\n",cantBlo);
    //Comprobación mi_truncar_f
    if(nbytes==0){
        liberar_inodo(ninodo);
    }
    else{
        mi_truncar_f(ninodo, nbytes);
    }
    
    struct STAT stats;
    mi_stat_f(ninodo,&stats);
    printf("Tamaño en bytes logicos: %d, numero bloques ocupados: %d\n",stats.tamEnBytesLog, stats.numBloquesOcupados);
    if(nbytes!=stats.numBloquesOcupados){
        printf("Error en truncar.c, al comprobar numBloquesOcupados\n");
        return -1;
    }
    //Desmontar dispositivo
    if(bumount()==-1){
        printf("Error en truncar.c, al ejecutar bumount\n");
        return -1;
    }
    return 0;
}