/* AUTORES:
 * Marc Nadal Sastre Gondar
 * Joaquín Esperon Solari
 * Martí Vich Gispert
 */

#include "verificacion.h"

void inicializar_info(struct INFORMACION *info, pid_t pid)
{
    info->pid = pid;
    info->nEscrituras = 0;
    time_t now = time(NULL);
    info->PrimeraEscritura.fecha = now;
    info->PrimeraEscritura.nEscritura = NUMESCRITURAS;
    info->PrimeraEscritura.nRegistro = 0;
    info->UltimaEscritura.fecha = 0;
    info->UltimaEscritura.nEscritura = 0;
    info->UltimaEscritura.nRegistro = 0;
    info->MenorPosicion.fecha = now;
    info->MenorPosicion.nEscritura = 0;
    info->MenorPosicion.nRegistro = REGMAX;
    info->MayorPosicion.fecha = now;
    info->MayorPosicion.nEscritura = 0;
    info->MayorPosicion.nRegistro = 0;
}

void escribir_info(int *infoPos, struct INFORMACION *info, const char *fic)
{
    char aux1[500];
    char aux2[100];

    memset(aux1, 0, sizeof(aux1));
    memset(aux2, 0, sizeof(aux2));

    sprintf(aux1, "PID: %d\n", info->pid);
    sprintf(aux2, "Numero escrituras: %d\n", info->nEscrituras);
    strcat(aux1, aux2);

    sprintf(aux2, "Primera escritura\t%d\t%d\t%s", info->PrimeraEscritura.nEscritura, info->PrimeraEscritura.nRegistro, asctime(localtime(&info->PrimeraEscritura.fecha)));
    strcat(aux1, aux2);

    sprintf(aux2, "Ultima escritura\t%d\t%d\t%s", info->UltimaEscritura.nEscritura, info->UltimaEscritura.nRegistro, asctime(localtime(&info->UltimaEscritura.fecha)));
    strcat(aux1, aux2);

    sprintf(aux2, "Menor Posicion\t\t%d\t%d\t%s", info->MenorPosicion.nEscritura, info->MenorPosicion.nRegistro, asctime(localtime(&info->MenorPosicion.fecha)));
    strcat(aux1, aux2);

    sprintf(aux2, "Mayor Posicion\t\t%d\t%d\t%s\n", info->MayorPosicion.nEscritura, info->MayorPosicion.nRegistro, asctime(localtime(&info->MayorPosicion.fecha)));
    strcat(aux1, aux2);

    *infoPos += mi_write(fic, aux1, *infoPos, strlen(aux1));
}

int main(int args, char *argv[])
{
    // Verificamos la sintaxis
    if (args != 3)
    {
        fprintf(stderr, RED "Sintaxis: ./verificacion <disco> <directorio_simulacion>\n" RESET);
        return FALLO;
    }

    char *nombre_dispositivo = argv[1];
    char *directorio = argv[2];

    // Montamos el dispositivo virtual
    if (bmount(nombre_dispositivo) == FALLO)
    {
        bumount();
        return FALLO;
    }

    struct STAT stat;

    // Calculamos el nº de entradas del directorio de simulación a partir del stat de su inodo. //100
    if (mi_stat(directorio, &stat) == FALLO)
    {
        fprintf(stderr, RED "Error al llamar a mi_stat \n" RESET);
        bumount();
        return FALLO;
    }

    int numentradas = stat.tamEnBytesLog / sizeof(struct entrada);

    fprintf(stderr, "\ndir_sim: %s \n", directorio);
    fprintf(stderr, "numEntradas: %d    NUMPROCESOS: %d \n", numentradas, NUMPROCESOS);

    // Si el nº de entradas != NUMPROCESOS entonces devolver FALLO fsi
    if (NUMPROCESOS != numentradas)
    {
        fprintf(stderr, RED "El numero de procesos es diferente al numero de entrada\n" RESET);
        bumount();
        return FALLO;
    }

    // Creamos el fichero "informe.txt" dentro del directorio de simulación
    char fic[256];
    strcpy(fic, directorio);
    strcat(fic, "informe.txt");

    if (mi_creat(fic, 6) == FALLO)
    {
        fprintf(stderr, RED "Error: no se pudo crear informe.txt\n" RESET);
        bumount();
        return FALLO;
    }

    // Leemos los directorios correspondientes a los procesos. //Entradas del directorio de simulación
    struct entrada entradas[NUMPROCESOS * sizeof(struct entrada)];
    if (mi_read(directorio, entradas, 0, NUMPROCESOS * sizeof(struct entrada)) == FALLO)
    {
        fprintf(stderr, RED "Error: No se pudieron leer las entradas\n" RESET);
        bumount();
        return FALLO;
    }

    struct INFORMACION info;
    int infoPos = 0;
    /*char aux1[500];
    char aux2[500];*/

    // Para cada entrada de directorio de un proceso...
    for (int i = 0; i < NUMPROCESOS; i++)
    {
        // Leemos la entrada de directorio
        char *pid = strchr(entradas[i].nombre, '_');
        if (!pid)
        {
            fprintf(stderr, RED "Error: nombre de entrada no tiene formato esperado\n" RESET);
            bumount();
            return FALLO;
        }
        pid++;

        // Extraer el PID e inicializar la información
        inicializar_info(&info, atoi(pid));

        /*
        // Extraemos el PID
        info.pid = atoi(pid);

        // Inicializar el resto de la estructura
        info.nEscrituras = 0;
        info.PrimeraEscritura.fecha = time(NULL);
        info.PrimeraEscritura.nEscritura = NUMESCRITURAS;
        info.PrimeraEscritura.nRegistro = 0;
        info.UltimaEscritura.fecha = 0;
        info.UltimaEscritura.nEscritura = 0;
        info.UltimaEscritura.nRegistro = 0;
        info.MenorPosicion.fecha = time(NULL);
        info.MenorPosicion.nEscritura = 0;
        info.MenorPosicion.nRegistro = REGMAX;
        info.MayorPosicion.fecha = time(NULL);
        info.MayorPosicion.nEscritura = 0;
        info.MayorPosicion.nRegistro = 0;
        */

        // Obtenemos el camino hacia cada prueba.dat
        // dirAux = /simul_aaaammddhhmmss/entradas[i].nombre/prueba.dat
        char dAux[256]; // direcotrio auxiliar
        strcpy(dAux, directorio);
        strcat(dAux, entradas[i].nombre);
        strcat(dAux, "/prueba.dat");

        // Creamos el buffer de N registros de escrituras
        int nRegBuff = 256;
        struct REGISTRO buffEscr[nRegBuff];
        int offset = 0;
        int read = 1;

        // Usamos el buffer para recorrer secuencialmente el fichero prueba.dat
        while (info.nEscrituras < NUMESCRITURAS && read > 0)
        {
            // Leemos una escritura
            read = mi_read(dAux, buffEscr, offset, sizeof(buffEscr));
            if (read < 0)
            {
                fprintf(stderr, RED "Error: No se pudo leer el fichero %s\n" RESET, dAux);
                bumount();
                return FALLO;
            }

            for (int j = 0; j < nRegBuff; j++)
            {
                // Si la escritura es válida entonces
                if (buffEscr[j].pid == info.pid)
                {
                    // Si es la primera escritura validada entonces
                    if (buffEscr[j].nEscritura < info.PrimeraEscritura.nEscritura)
                    {
                        // Inicilizamos los registros significativos con los datos de esa escritura
                        info.PrimeraEscritura.fecha = buffEscr[j].fecha;
                        info.PrimeraEscritura.nEscritura = buffEscr[j].nEscritura;
                        info.PrimeraEscritura.nRegistro = buffEscr[j].nRegistro;
                    }
                    else
                    {
                        double diff_t = difftime(info.UltimaEscritura.fecha, buffEscr[j].fecha);
                        if (diff_t < 0)
                        {
                            info.UltimaEscritura.fecha = buffEscr[j].fecha;
                            info.UltimaEscritura.nEscritura = buffEscr[j].nEscritura;
                            info.UltimaEscritura.nRegistro = buffEscr[j].nRegistro;
                        }
                        else if (diff_t == 0)
                        {
                            if (buffEscr[j].nEscritura > info.UltimaEscritura.nEscritura)
                            {
                                info.UltimaEscritura.fecha = buffEscr[j].fecha;
                                info.UltimaEscritura.nEscritura = buffEscr[j].nEscritura;
                                info.UltimaEscritura.nRegistro = buffEscr[j].nRegistro;
                            }
                        }
                    }
                    // Miramos la mayor y menor de las escrituras con nRegistro
                    if (buffEscr[j].nRegistro < info.MenorPosicion.nRegistro)
                    {
                        info.MenorPosicion.fecha = buffEscr[j].fecha;
                        info.MenorPosicion.nEscritura = buffEscr[j].nEscritura;
                        info.MenorPosicion.nRegistro = buffEscr[j].nRegistro;
                    }
                    if (buffEscr[j].nRegistro > info.MayorPosicion.nRegistro)
                    {
                        info.MayorPosicion.fecha = buffEscr[j].fecha;
                        info.MayorPosicion.nEscritura = buffEscr[j].nEscritura;
                        info.MayorPosicion.nRegistro = buffEscr[j].nRegistro;
                    }

                    // Incrementamos el contador de escrituras validadas.
                    info.nEscrituras++;
                }
            }
            // Leemos siguiente escritura
            offset += read;
            memset(buffEscr, 0, sizeof(buffEscr));
        }

        /*
        // Añadir la información del struct info al fichero informe.txt por el final.
        memset(aux1, 0, 500);
        memset(aux2, 0, 500);
        sprintf(aux1, "PID: %d\n", info.pid);
        sprintf(aux2, "Numero escrituras: %d\n", info.nEscrituras);
        strcat(aux1, aux2);
        memset(aux2, 0, 100);
        sprintf(aux2, "Primera escritura\t%d\t%d\t%s", info.PrimeraEscritura.nEscritura, info.PrimeraEscritura.nRegistro, asctime(localtime(&info.PrimeraEscritura.fecha)));
        strcat(aux1, aux2);
        memset(aux2, 0, 100);
        sprintf(aux2, "Ultima escritura\t%d\t%d\t%s", info.UltimaEscritura.nEscritura, info.UltimaEscritura.nRegistro, asctime(localtime(&info.UltimaEscritura.fecha)));
        strcat(aux1, aux2);
        memset(aux2, 0, 100);
        sprintf(aux2, "Menor Posicion\t\t%d\t%d\t%s", info.MenorPosicion.nEscritura, info.MenorPosicion.nRegistro, asctime(localtime(&info.MenorPosicion.fecha)));
        strcat(aux1, aux2);
        memset(aux2, 0, 100);
        sprintf(aux2, "Mayor Posicion\t\t%d\t%d\t%s\n", info.MayorPosicion.nEscritura, info.MayorPosicion.nRegistro, asctime(localtime(&info.MayorPosicion.fecha)));
        strcat(aux1, aux2);

        infoPos += mi_write(fic, aux1, infoPos, strlen(aux1));
        */

        escribir_info(&infoPos, &info, fic);
        fprintf(stderr, "[%d) %d escrituras validadas en %s]\n", i + 1, info.nEscrituras, dAux);
    }

    // Desmontamos el dispositivo virtual
    if (bumount() == FALLO)
    {
        return FALLO;
    }
    return EXITO;
}
