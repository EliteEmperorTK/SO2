/* AUTORES:
 * Marc Nadal Sastre Gondar
 * Joaquín Esperon Solari
 * Martí Vich Gispert
 */

#include "simulacion.h"

static int acabados = 0;
/*
 * Simulación que crea procesos que acceden de forma concurrente para comprobar que funcionan correctamente
 * args: cantidad de argumentos
 * argv: lista de argumentos: [0] = nombre del dispositivo
 */
int main(int args, char *argv[])
{
    pid_t pid;
    time_t t;
    struct tm tm;
    char tmp[100];
    char camino[200], direc2[50];
    struct REGISTRO registro;
    int nescritura;

    // Asociar la señal SIGCHLD al enterrador
    signal(SIGCHLD, reaper);

    // Comprobar sintaxis
    if (args != 2)
    {
        fprintf(stderr, "Sintaxis ./simulacion <nombre_dispositivo>\n");
        return FALLO;
    }

    // Montar el dispositivo
    bmount(argv[1]);

    // Crear directorio de simulacion
    t = time(NULL);
    tm = *localtime(&t);
    sprintf(tmp, "%d%02d%02d%02d%02d%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    sprintf(camino, "/simul_%s/", tmp);

    if (mi_creat(camino, 6) == FALLO)
    {
        bumount();
        return FALLO;
    }

    fprintf(stderr, "*** SIMULACIÓN DE %d PROCESOS REALIZANDO CADA UNO %d ESCRITURAS ***\n", NUMPROCESOS, NUMESCRITURAS);

    for (int proceso = 1; proceso <= NUMPROCESOS; proceso++)
    {
        pid = fork();

        if (pid == 0)
        { // Es el hijo
            // Montar dispositivo
            if (bmount(argv[1]) == FALLO)
            { // Hijo
                return FALLO;
            }

            // Crear directorio del proceso añadiendo el PID al nombre
            sprintf(direc2, "proceso_%d/", getpid());
            strcat(camino, direc2);
            mi_creat(camino, 6);

            // Crear fichero prueba.dat
            strcat(camino, "prueba.dat");
            if (mi_creat(camino, 6) == FALLO)
            {
                fprintf(stderr, "Error creando prueba.dat\n");
                bumount();
                return FALLO;
            }

            // Inicializar semilla de nº aleatorios
            srand(time(NULL) + getpid());

            for (nescritura = 1; nescritura <= NUMESCRITURAS; nescritura++) // 50
            {
                // Inicializar el registro
                registro.fecha = time(NULL);
                registro.pid = getpid();
                registro.nEscritura = nescritura;
                registro.nRegistro = rand() % REGMAX;

                // Escribir registro
                mi_write(camino, &registro, registro.nRegistro * sizeof(struct REGISTRO), sizeof(struct REGISTRO));
                // fprintf(stderr, "[simulación.c → Escritura %d en %s]\n", nescritura, camino);

                // my_sleep(50); // Esperamos 0,05 antes de la siguiente escritura
                usleep(50000);
            }

            fprintf(stderr, "[Proceso %d: Completadas %d escrituras en %s]\n", proceso, nescritura - 1, camino);

            bumount(); // Se desmonta el dispositivo HIJO

            exit(0); // Necesario para que se emita la señal SIGCHLD
        }
        // my_sleep(150); // Esperar 0,15 segundos para lanzar el siguiente proceso
        usleep(150000);
    }

    // Permitir que el padre espere por todos los hijos
    while (acabados < NUMPROCESOS)
    {
        pause();
    }

    bumount(); // Desmontar el dispositivo PADRE
    return EXITO;
}

/**
 * Enterrador
 */
void reaper()
{
    pid_t ended;
    signal(SIGCHLD, reaper);
    while ((ended = waitpid(-1, NULL, WNOHANG)) > 0)
    {
        acabados++;
    }
}

/* Convierte milisegundos en nanosegundos para hacer una espera */
/*void my_sleep(unsigned msec)
{ // recibe tiempo en milisegundos
    struct timespec req, rem;
    int err;
    req.tv_sec = msec / 1000;              // conversión a segundos
    req.tv_nsec = (msec % 1000) * 1000000; // conversión a nanosegundos
    while ((req.tv_sec != 0) || (req.tv_nsec != 0))
    {
        if (nanosleep(&req, &rem) == 0)
            // rem almacena el tiempo restante si una llamada al sistema
            // ha sido interrumpida por una señal
            break;
        err = errno;
        // Interrupted; continue
        if (err == EINTR)
        {
            req.tv_sec = rem.tv_sec;
            req.tv_nsec = rem.tv_nsec;
        }
    }
}*/
