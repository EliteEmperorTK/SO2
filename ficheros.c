#include "ficheros.h"


int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes){ //Escribimos en un fichero. Devuelve la cantidad de Bytes escritos
    
    int cant_bytes;
    struct inodo inodos;

    if(leer_inodo(ninodo,&inodos) == FALLO){ //Leemos el inodo
        perror(RED "Error en mi_write_f: No se ha podido leer el inodo.\n" RESET);
        return FALLO;
    }

    if ((inodos.permisos & 2) != 2) { //Miramos si el inodo tiene permisos de escritura
       perror(RED "Error en mi_write_f: No hay permisos de escritura.\n" RESET);
       return FALLO;
    }

    int primerBL = offset / BLOCKSIZE;                  //Primer bloque lógico donde escribimos
    int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;   //Último bloque lógico donde escribimos
    int desp1 = offset % BLOCKSIZE;                     //Desplazamiento con el offset
    int desp2 = (offset + nbytes - 1) % BLOCKSIZE;      //Desplazamiento nbytes después del offset


    int nbfisico = traducir_bloque_inodo(&inodos, primerBL, 1); //Encontramos la posición real (común en ambos casos)
    if(nbfisico == FALLO){
        perror(RED "Error en mi_write_f: No se ha podido encontrar la posición real del primer bloque físico.\n" RESET);
        return FALLO;
    }

    unsigned char buf_bloque[BLOCKSIZE];
    if(bread(nbfisico,&buf_bloque) == FALLO){ //Leemos el bloque y lo metemos en un buffer (común en ambos casos)
        perror(RED "Error en mi_write_f: No se ha podido leer el primer bloque físico. \n" RESET);
        return FALLO;
    }
    
    //2 Casos
    if(primerBL == ultimoBL){ //Caso 1 (Escritura en 1 solo bloque)

        memcpy(buf_bloque + desp1, buf_original, nbytes);   //Escribimos nbytes del buf_original en la posición (buf_bloque + desp1)

        cant_bytes = bwrite(nbfisico,buf_bloque); //Escribimos lo del buffer donde toca
        if(cant_bytes == FALLO){
            perror(RED "Error en mi_write_f Caso 1: No se ha podido escribir.\n" RESET);
            return FALLO;
        }


    } else{ //Caso 2 (Escritura en más de 1 bloque)

        //Fase 1: Escribimos el primer bloque
        memcpy(buf_bloque + desp1, buf_original, BLOCKSIZE-desp1);

        cant_bytes = bwrite(nbfisico,buf_bloque); //Escribimos lo del buffer donde toca
        if(cant_bytes == FALLO){
            perror(RED "Error en mi_write_f Caso 2: No se ha podido escribir en el primer bloque.\n" RESET);
            return FALLO;
        }

        //Fase 2: Escribimos los bloques intermedios
        
        for(int bl = primerBL+1; bl < ultimoBL; bl++){  /////////////////////////LA CONDICIÓN PODRÍA ESTAR MAL (no creo).  //Recorremos todos menos el último/////////////////////////
                               //////////////////////EL -1 DEL BWRITE PODRÍA SOBRAR (AUNQUE LO PONE LA MAESTRA)//////////////////////////////////////////////////////

            nbfisico = traducir_bloque_inodo(&inodos,bl,1); //Obtenemos la posición del nuevo bloque
            if(nbfisico == FALLO){

            }

            int aux = bwrite(nbfisico, buf_original + (BLOCKSIZE - desp1) + (bl - primerBL - 1) * BLOCKSIZE); //Escribimos el bloque nº bl entero
            if(aux == FALLO){
                perror(RED "Error en mi_write_f Caso 2: No se ha podido escribir en un bloque intermedio.\n" RESET);
                return FALLO;
                break;
            }

            cant_bytes += aux; //Aumentamos la cantidad de bytes escritos
        }

        //Fase 3: Escribimos el último bloque
        nbfisico = traducir_bloque_inodo(&inodos,ultimoBL,1); //Obtenemos la posición del nuevo bloque
        if(nbfisico == FALLO){

        }
        
        if(bread(nbfisico,&buf_bloque) == FALLO){ //Leemos el bloque y lo metemos en un buffer

        }

        memcpy(buf_bloque, buf_original + (nbytes - (desp2 + 1)), desp2 + 1);

        int aux = bwrite(nbfisico, buf_bloque);
        if(aux == FALLO){
            perror(RED "Error en mi_write_f Caso 2: No se ha podido escribir en el último bloque.\n" RESET);
            return FALLO;
        }
        cant_bytes += aux;
    }

    //Actualizamos la información del inodo
    if(inodos.tamEnBytesLog < offset + cant_bytes){ //Si hemos escrito más allá del tamEnBytesLog, lo actualizamos como nuevo tamaño máximo. /////////////////PODRÍA ESTAR MAL //////////////////////////
        inodos.tamEnBytesLog = offset + cant_bytes;
    }

    inodos.ctime = time(NULL); //hora actual
    inodos.mtime = time(NULL);

    if(escribir_inodo(ninodo, &inodos) == FALLO){ //Escribimos el inodo actualizado
        perror(RED "Error en mi_write_f: No se ha podido escribir el inodo actualizado.\n" RESET);
        return FALLO;
    }
    return cant_bytes;
}




int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes){ //Leemos un fichero. buf_original debe estar inicializado a 0's
    int cant_bytes = 0;
    struct inodo inodos;

    if(leer_inodo(ninodo,&inodos) == FALLO){ //Leemos el inodo
        perror(RED "Error en mi_read_f: No se ha podido leer el inodo.\n" RESET);
        return FALLO;
    }

    if ((inodos.permisos & 4) != 4) { //Miramos si el inodo tiene permisos de escritura
       perror(RED "Error en mi_read_f: No hay permisos de escritura.\n" RESET);
       return FALLO;
    }

    if(offset >= inodos.tamEnBytesLog){  //Si ya se encuentra fuera del EOF (End Of File)
       cant_bytes = 0;                   // No podemos leer nada
       return cant_bytes;
    }

    if((offset + nbytes) >= inodos.tamEnBytesLog){  //Si pretende leer más allá de EOF
       nbytes = inodos.tamEnBytesLog - offset;      //Leemos sólo hasta el EOF 
    }


    int primerBL = offset / BLOCKSIZE;                  //Primer bloque lógico donde leemos
    int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;   //Último bloque lógico donde leemos
    int desp1 = offset % BLOCKSIZE;                     //Desplazamiento con el offset
    int desp2 = (offset + nbytes - 1) % BLOCKSIZE;      //Desplazamiento nbytes después del offset

    unsigned char buf_bloque[BLOCKSIZE];                //Buffer para leer antes de meterlo en el original

    //Encontramos la posición del primer bloque lógico (común en ambos casos)
    int nbfisico = traducir_bloque_inodo(&inodos, primerBL, 0); //Encontramos la posición real

    if(nbfisico == FALLO){ //Si vale -1 no existe y hay que saltar el bloque
        cant_bytes += BLOCKSIZE;

    }else{
        cant_bytes += bread(nbfisico,&buf_bloque); //Leemos el bloque y lo metemos en un buffer

        if(cant_bytes == FALLO){
            perror(RED "Error en mi_read_f: No se ha podido leer el primer bloque físico. \n" RESET);
            return FALLO;
        }
    }
    
    //2 Casos
    if(primerBL == ultimoBL){ //Caso 1 (Lectura de 1 solo bloque [ya lo tenemos leído de antes])

        if(nbfisico == FALLO){
            //No hacemos nada porque ya lo hemos tratado anteriormente

        } else{ //Hemos leído bloque -> copiamos la info en buf_original

        memcpy(buf_original + desp1, buf_bloque, nbytes); //Escribimos nbytes del buf_bloque en la posición (buf_original+ desp1)
        //////////////////////////////////////////////////////LO MISMO LOS PARÁMETROS DEL MEMCOPY ESTÁN MAL/////////////////////////////////////////////////////////
        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        }
//Fin Caso 1

    } else{ //Caso 2 (Lectura de más de 1 bloque)

        //Fase 1: Acabamos de leer el primer bloque (copiamos la info)
        memcpy(buf_original + desp1, buf_bloque, BLOCKSIZE-desp1);


        //Fase 2: Leemos los bloques intermedios       
        for(int bl = primerBL+1; bl < ultimoBL; bl++){  /////////////////////////LA CONDICIÓN PODRÍA ESTAR MAL (no creo).  //Recorremos todos menos el último/////////////////////////
                               //////////////////////EL -1 DEL BREAD PODRÍA SOBRAR (AUNQUE LO PONE LA MAESTRA)//////////////////////////////////////////////////////

            nbfisico = traducir_bloque_inodo(&inodos,bl,0); //Obtenemos la posición del nuevo bloque
                       
            if(nbfisico == FALLO){ //No existe el bloque

                cant_bytes += BLOCKSIZE;

            } else{ //Existe el bloque
                int aux = bread(nbfisico,&buf_bloque); //Leemos el bloque y lo metemos en un buffer

                if(aux == FALLO){
                    perror(RED "Error en mi_read_f Caso 2: No se ha podido leer un bloque intermedio. \n" RESET);
                    return FALLO;
                }
                cant_bytes += aux;      //Aumentamos la cantidad de bytes leídos

                memcpy(buf_original + desp1, buf_bloque, BLOCKSIZE);
            }
        }

        //Fase 3: Leemos el último bloque
        nbfisico = traducir_bloque_inodo(&inodos,ultimoBL,0); //Obtenemos la posición del nuevo bloque

        if(nbfisico == FALLO){ //No existe el bloque
            cant_bytes += BLOCKSIZE;

        } else{ //Existe el bloque
                int aux = bread(nbfisico,&buf_bloque); //Leemos el bloque y lo metemos en un buffer

                if(aux == FALLO){
                    perror(RED "Error en mi_read_f Caso 2: No se ha podido leer el último bloque %d. \n" RESET);
                    return FALLO;
                }
                cant_bytes += aux;      //Aumentamos la cantidad de bytes leídos

                memcpy(buf_original + nbytes - desp2 + 1, buf_bloque, desp2 + 1);
            }
    } //Fin Caso 2

        //Actualizamos la información del inodo
    inodos.atime = time(NULL); //hora actual

    if(escribir_inodo(ninodo, &inodos) == FALLO){ //Escribimos el inodo actualizado
        perror(RED "Error en mi_write_f: No se ha podido escribir el inodo actualizado.\n" RESET);
        return FALLO;
    }

    return cant_bytes;
}


int mi_stat_f(unsigned int ninodo, struct STAT *p_stat)
{
    struct inodo inodo;

    // Leer el inodo correspondiente al número de inodo pasado como argumento
    if (leer_inodo(ninodo, &inodo) == -1)
    {
        return -1; // Error al leer el inodo
    }

    // Asignar los valores de los metadatos del inodo al struct STAT pasado por referencia
    p_stat->tipo = inodo.tipo;
    p_stat->permisos = inodo.permisos;
    p_stat->atime = inodo.atime;
    p_stat->mtime = inodo.mtime;
    p_stat->ctime = inodo.ctime;
    p_stat->nlinks = inodo.nlinks;
    p_stat->tamEnBytesLog = inodo.tamEnBytesLog;
    p_stat->numBloquesOcupados = inodo.numBloquesOcupados;

    return 0; // Éxito
}

int mi_chmod_f(unsigned int ninodo, unsigned char permisos)
{
    struct inodo inodo;

    // Leer el inodo correspondiente al número de inodo pasado como argumento
    if (leer_inodo(ninodo, &inodo) == -1)
    {
        return -1; // Error al leer el inodo
    }

    // Cambiar los permisos del inodo
    inodo.permisos = permisos;
    inodo.ctime = time(NULL); // Actualizar ctime

    // Escribir el inodo modificado en el dispositivo
    if (escribir_inodo(ninodo, &inodo) == -1)
    {
        return -1; // Error al escribir el inodo modificado
    }

    return 0; // Éxito
}



int mi_truncar_f(unsigned int ninodo, unsigned int nbytes){

    int bloq_liber;
    int primerBL;           //Bloque lógico desde el cual empezamos a liberar
    struct inodo inodos;

    if(leer_inodo(ninodo,&inodos) == FALLO){ //Leemos el inodo a truncar

    }


    if(!(inodos.permisos & 2)){ //Comprobamos si tiene permisos de escritura (entra si no tiene)

    }


    if(nbytes > inodos.tamEnBytesLog){  //No podemos truncar más allá del EOF, por tanto
        nbytes = inodos.tamEnBytesLog;  //hacemos que nbytes sea el EOF
    }


    if((nbytes % BLOCKSIZE) == 0){ //Comprobamos cuál debe ser el primer bloque lógico
        primerBL = nbytes/BLOCKSIZE;

    }else{ 
        primerBL = nbytes/BLOCKSIZE + 1;
    }


    bloq_liber = liberar_bloques_inodo(primerBL,&inodos); //Utilizamos la función liberar bloques inodo
    if( bloq_liber == FALLO){

    }


    //Actualizamos la información del inodo y lo escribimos
    inodos.ctime = time(NULL); //hora actual
    inodos.mtime = time(NULL);
    inodos.tamEnBytesLog = nbytes;
    inodos.numBloquesOcupados -= bloq_liber; //Restamos los bloques liberados al total de bloques ocupados

    if(escribir_inodo(ninodo, &inodos) == FALLO){ //Guardamos el inodo

    }
}

