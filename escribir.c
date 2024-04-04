#include "ficheros.h"


int main(int args, char** argv){
    //Validación de argumentos
    if(args!= 4){
        printf("Sintaxis introducida incorrecta, sintaxis correcta: './escribir <nombre_dispositivo><""$(cat fichero)""><diferentes_inodos>");
        return -1;
    }
    //Inicialización argumentos
    char* nombre_dispositivo=argv[1];
    char *fichero=argv[2];
    int diferentes_inodos= atoi(argv[3]);
    int lon=strlen(fichero);
    int OFFSETS[5]={9000, 209000, 30725000, 409605000, 480000000};
    char buffer [lon];
    //copia de fichero dado por parametro a buffer para manipulación
    strcpy(buffer, fichero);
    //montar dispositivo
    if(bmount(nombre_dispositivo)==-1){
        printf("Error en escribir.c al ejecutar bmount()");
        return -1;
    }
    struct STAT stats;
    int nInodo = reservar_inodo('f',6);
	printf("longitud texto: %d\n", lon);
	int bytesEscritos = mi_write_f(nInodo,fichero,OFFSETS[0],lon);
	memset(buffer,0,lon);
	mi_stat_f(nInodo, &stats);
    for (int i = 0; i < 5; ++i){
		if (diferentes_inodos!=0){
			nInodo = reservar_inodo('f',6);
		}
		bytesEscritos=mi_write_f(nInodo,fichero,OFFSETS[i],lon);
		memset(buffer,0,lon);
		mi_stat_f(nInodo,&stats);
		printf("\nNº inodo reservado: %d\noffset: %d\nBytes escritos: %d\n\n",nInodo,OFFSETS[i],bytesEscritos);
		printf("Tamaño en bytes lógicos: %d\n", stats.tamEnBytesLog);
		printf("N. de bloques ocupados: %d\n", stats.numBloquesOcupados);
	}
    if(bumount()==-1){
		fprintf(stderr, "Error en escribir.c --> %d: %s\n", errno, strerror(errno));
		return -1;
	}
}