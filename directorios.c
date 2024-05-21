/* AUTORES:
 * Marc Nadal Sastre Gondar
 * Joaquín Esperon Solari
 * Martí Vich Gispert
 */

#include "directorios.h"

// NIVEL 7

/* Muestra por pantalla un error
 * error: tipo de error, dependiendo de su valor se lanza un error u otro.
 */
void mostrar_error_buscar_entrada(int error)
{
    switch (error)
    {
    case -2:
        fprintf(stderr, RED "Error: Camino incorrecto.\n" RESET);
        break;
    case -3:
        fprintf(stderr, RED "Error: Permiso denegado de lectura.\n" RESET);
        break;
    case -4:
        fprintf(stderr, RED "Error: No existe el archivo o el directorio.\n" RESET);
        break;
    case -5:
        fprintf(stderr, RED "Error: No existe algún directorio intermedio.\n" RESET);
        break;
    case -6:
        fprintf(stderr, RED "Error: Permiso denegado de escritura.\n" RESET);
        break;
    case -7:
        fprintf(stderr, RED "Error: El archivo ya existe.\n" RESET);
        break;
    case -8:
        fprintf(stderr, RED "Error: No es un directorio.\n" RESET);
        break;
    }
}

/* Dada una cadena de caracteres se separa su contenido en una parte inicial y una parte final.
 * camino: cadena de caracteres inicial, empieza por '/'
 * inicial: al final será la porción comprendida entre las dos primeras '/'. Si no hay segundo '/', tendrá el nombre del fichero
 * final: resto del camino a partir del segundo '/' (inclusive)
 * tipo: d o f en función de si camino es un directorio o un fichero
 */
int extraer_camino(const char *camino, char *inicial, char *final, char *tipo)
{
    // Verificar que el camino no sea nulo y que empiece con '/'
    if (camino == NULL || *camino != '/')
    {
        mostrar_error_buscar_entrada(ERROR_CAMINO_INCORRECTO);
        return ERROR_CAMINO_INCORRECTO;
    }

    int segunda_barra = 0; // Variable para indicar si se encontró la segunda barra

    // Iteramos todo el camino empezando desde el segundo caracter (para saltarnos la barra [ya la hemos comprobado antes])
    for (int i = 1; i < strlen(camino); i++)
    {
        // Si se encuentra una barra '/'
        if (camino[i] == '/')
        {
            // Extraemos la parte de antes de la segunda barra en *inicial
            strncpy(inicial, camino + 1, i - 1); // Guardamos el resto del camino después de la primera barra

            // Extraemos la parte de después de la segunda barra en *final
            strcpy(final, camino + i);

            segunda_barra = 1; // Indicar que se encontró la segunda barra
            *tipo = 'd';       // Indicar que es un directorio
            return EXITO;
        }
    }

    // Si no se encontró la segunda barra, se asume que el camino solo contiene el nombre de un fichero
    if (segunda_barra == 0)
    {
        // Extraer el nombre del fichero en *inicial
        strncpy(inicial, camino + 1, strlen(camino) - 1);

        // Indicar que es un fichero
        *tipo = 'f';

        return EXITO; // Devolver éxito
    }

    return ERROR_CAMINO_INCORRECTO; // Si no se encontró la segunda barra, devuelve error
}

/* Función recursiva que busca una determinada entrada entre las entradas del inodo correspondiente a su directorio padre
 * camino_parcial: cadena de caracteres que representa la parte inicial de un camino
 * p_inodo_dir: nº inodo del directorio padre
 * p_inodo: nº inodo al que está asociado el nombre de la entrada buscada
 * p_entrada: nº de entrada dentro del inodo *p_inodo_dir que lo contiene (empezando por 0)
 * reservar: para saber si tan solo consultamos (0) o si también crear una entrada de directorio (1)
 * permisos: los permisos que asignamos a los inodos que reservemos
 */
int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos)
{
    struct entrada entrada;
    struct inodo inodo_dir;

    char inicial[sizeof(entrada.nombre)];
    char final[strlen(camino_parcial)];

    char tipo;
    int cant_entradas_inodo;
    int num_entrada_inodo;

    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO)
    {
        fprintf(stderr, RED "Error al leer el superbloque.\n" RESET);
        return FALLO;
    }

    if (strcmp(camino_parcial, "/") == 0) // el camino parcial es '/'
    {
        *p_inodo = SB.posInodoRaiz; // nuestra raiz siempre estará asociada al inodo 0
        *p_entrada = 0;
        return EXITO;
    }

    memset(inicial, 0, sizeof(entrada.nombre)); // Inicializamos el contenido de los buffer
    memset(final, 0, strlen(camino_parcial) + 1);

    if (extraer_camino(camino_parcial, inicial, final, &tipo) == FALLO)
    {
        return ERROR_CAMINO_INCORRECTO;
    }

    // fprintf(stderr, "[buscar_entrada()→ inicial: %s, final: %s, reservar: %d]\n", inicial, final, reservar);

    //  buscamos la entrada cuyo nombre se encuentra en inicial
    if (leer_inodo(*p_inodo_dir, &inodo_dir) == FALLO)
    {
        fprintf(stderr, RED "Error al leer el inodo en buscar_entrada" RESET);
        return FALLO;
    }

    if ((inodo_dir.permisos & 4) != 4) // Miramos si el inodo tiene permisos de lectura
    {
        // fprintf(stderr, GRAY "[buscar_entrada()→ El inodo 2 no tiene permisos de lectura]\n" RESET);
        return ERROR_PERMISO_LECTURA;
    }

    cant_entradas_inodo = inodo_dir.tamEnBytesLog / sizeof(struct entrada);
    num_entrada_inodo = 0; // nº de entrada inicial;

    // inicializar el buffer de lectura con 0s
    memset(entrada.nombre, 0, sizeof(entrada.nombre));

    // el buffer de lectura puede ser un struct tipo entrada
    // o mejor un array de las entradas que caben en un bloque, para optimizar la lectura en RAM

    if (cant_entradas_inodo > 0)
    {
        // leer entrada
        if (mi_read_f(*p_inodo_dir, &entrada, 0, sizeof(struct entrada)) == FALLO)
        {
            fprintf(stderr, RED "Error al leer entrada en buscar_entrada" RESET);
            return FALLO;
        }
        while ((num_entrada_inodo < cant_entradas_inodo) && (strcmp(inicial, entrada.nombre) != 0))
        {
            num_entrada_inodo++;

            // limpiar buffer de lectura con 0s
            memset(entrada.nombre, 0, sizeof(entrada.nombre));
            // leer siguiente entrada
            if (mi_read_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada)) < 0)
            {
                fprintf(stderr, RED "Error al leer entrada en buscar_entrada" RESET);
                return FALLO;
            }
        }
    }

    if (strcmp(inicial, entrada.nombre) != 0 && (num_entrada_inodo == cant_entradas_inodo))
    {
        switch (reservar)
        {
        case 0: // Modo consulta. Como no existe, retornamos error
            return ERROR_NO_EXISTE_ENTRADA_CONSULTA;

        case 1: // modo escritura
            // Creamos la entrada en el directorio referenciado por *p_inodo_dir
            // si es fichero no permitir escritura
            if (inodo_dir.tipo == 'f')
            {
                return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO;
            }

            // si es directorio comprobar que tiene permiso de escritura
            if ((inodo_dir.permisos & 2) != 2)
            { // No tiene permisos de escritura
                return ERROR_PERMISO_ESCRITURA;
            }
            else
            {
                strcpy(entrada.nombre, inicial);

                if (tipo == 'd')
                {
                    // Si el siguiente empieza por /
                    if (strcmp(final, "/") == 0)
                    {
                        // reservar un inodo como directorio y asignarlo a la entrada
                        entrada.ninodo = reservar_inodo('d', permisos);
                        // fprintf(stderr, "[buscar_entrada()→ reservado inodo %d tipo %c con permisos %d para %s]\n", entrada.ninodo, tipo, permisos, inicial);
                    }
                    else
                    {
                        return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
                    }
                }
                else // es un fichero
                {
                    // reservar un inodo como fichero y asignarlo a la entrada
                    entrada.ninodo = reservar_inodo('f', permisos);
                    // fprintf(stderr, "[buscar_entrada()→ reservado inodo %d tipo %c con permisos %d para %s]\n", entrada.ninodo, tipo, permisos, inicial);
                }
                // escribir la entrada en el directorio padre
                if (mi_write_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada)) == FALLO)
                {                             // error de escritura
                    if (entrada.ninodo != -1) // si habiamos reservado un inodo
                    {
                        // liberar el inodo
                        liberar_inodo(entrada.ninodo);
                        // fprintf(stderr, "[buscar_entrada()-> liberado inodo %i, reservado a %s]\n", num_entrada_inodo, inicial);
                    }
                    else
                    {
                        return FALLO;
                    }
                }
                // fprintf(stderr, "[buscar_entrada()-> creada entrada: %s, %d]\n", entrada.nombre, entrada.ninodo);
            }
        }
    }
    if ((strcmp(final, "") == 0) || (strcmp(final, "/") == 0))
    { // hemos llegado al final del camino
        if ((num_entrada_inodo < cant_entradas_inodo) && (reservar == 1))
        {
            // modo escritura y la entrada ya existe
            return ERROR_ENTRADA_YA_EXISTENTE;
        }
        // cortamos la recursividad
        // asignar a *p_inodo el numero de inodo del directorio o fichero creado o leido
        *p_inodo = entrada.ninodo;
        // asignar a *p_entrada el número de su entrada dentro del último directorio que lo contiene
        *p_entrada = num_entrada_inodo;
        return EXITO;
    }
    else
    {
        // asignamos a *p_inodo_dir el puntero al inodo que se indica en la entrada encontrada;
        *p_inodo_dir = entrada.ninodo;
        return buscar_entrada(final, p_inodo_dir, p_inodo, p_entrada, reservar, permisos);
    }
    return EXITO;
}

// NIVEL 8

/* Crea un fichero/directorio y su entrada de directorio
 * camino: nos indica la ruta hasta el fichero/directorio
 * permisos: los permisos que asignamos a los inodos que reservemos
 */
int mi_creat(const char *camino, unsigned char permisos)
{
    mi_waitSem();

    int posible_error;

    // Declaramos los punteros
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;

    posible_error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 1, permisos); // Llamamos a buscar_entrada con reservar = 1

    if (posible_error < 0) // Si hay un error, lo mostramos por pantalla
    {
        mostrar_error_buscar_entrada(posible_error);
        mi_signalSem();
        return FALLO;
    }
    mi_signalSem();
    return EXITO;
}

/* Pone el contenido del directorio(camino) en un buffer de memoria(buffer)
 * camino: nos indica la ruta hasta el fichero/directorio
 * buffer: buffer de memoria que recibe  el directorio
 * extra:  extra = 0 -> lista el nombre de las entradas de directorio. extra = 1 -> lista también más información
 */
int mi_dir(const char *camino, char *buffer, char tipo, int flag)
{
    struct superbloque SB;
    struct inodo inodo;
    struct entrada entrada;

    char tmp[100];

    int numEntradas = 0; // Cantidad de entradas del inodo
    int posibleError;

    if (bread(posSB, &SB) == FALLO)
    {
        fprintf(stderr, RED "Error al leer el superbloque en el disco.\n" RESET);
        return FALLO;
    }

    unsigned int p_entrada = 0;
    unsigned int p_inodo_dir = SB.posInodoRaiz;
    unsigned int posInodoRaiz = SB.posInodoRaiz;

    // Buscamos la entrada
    posibleError = buscar_entrada(camino, &p_inodo_dir, &posInodoRaiz, &p_entrada, 0, 0);

    if (posibleError < 0) // Si hay un error lo enseñamos por pantalla
    {
        mostrar_error_buscar_entrada(posibleError);
        return FALLO;
    }
    else
    {
        leer_inodo(posInodoRaiz, &inodo); // Leemos el inodo
        numEntradas = inodo.tamEnBytesLog / sizeof(struct entrada);

        // agregamos esto
        if (tipo != inodo.tipo) // Si no coinciden los tipos, es un error
        {
            // fprintf(stderr, "Tipo mi_ls = %c    Tipo mi_dir = %c", tipo, inodo.tipo);
            fprintf(stderr, RED "Error: la sintaxis no concuerda con el tipo." RESET);
            return FALLO;
        }

        if ((inodo.permisos & 4) != 4) // Comprobamos que el inodo tenga permisos de lectura
        {
            mostrar_error_buscar_entrada(ERROR_PERMISO_LECTURA);
            return ERROR_PERMISO_LECTURA;
        }

        // Podéis hacer uso del dato *p_entrada que os ha devuelto buscar_entrada() y
        // emplearlo en mi_read_f() para obtener el offset adecuado para leer directamente esa entrada y obtener el nombre del fichero

        // Recorremos todas las entradas
        if (flag == 0) // CASO SIN -l
        {
            // Si la ruta pasada corresponde a un fichero -> tan solo leemos este fichero
            if (tipo == 'f')
            {
                memset(entrada.nombre, 0, sizeof(entrada.nombre)); // Inicializamos el buffer de lectura poniendo zeros

                if (mi_read_f(p_inodo_dir, &entrada, p_entrada, sizeof(struct entrada)) == FALLO) // Leemos el fichero correspondiente
                {
                    fprintf(stderr, RED "No ha podido leerse el fichero en mi_dir." RESET);
                    return FALLO;
                }

                // color azul claro
                strcat(buffer, LBLUE);
                strcat(buffer, entrada.nombre);
                strcat(buffer, RESET);
                strcat(buffer, "\t"); // Agregamos una tabulación entre cada entrada
                // FICHEROS
                return numEntradas;
            }

            // La ruta pasada corresponde a un directorio -> leemos todas sus entradas
            for (int idx = 0; idx < numEntradas; idx++)
            {
                memset(entrada.nombre, 0, sizeof(entrada.nombre)); // Inicializamos el buffer de lectura poniendo zeros

                if (mi_read_f(posInodoRaiz, &entrada, idx * sizeof(struct entrada), sizeof(struct entrada)) == FALLO) // Leemos el fichero correspondiente
                {
                    fprintf(stderr, RED "No ha podido leerse el fichero en mi_dir." RESET);
                    return FALLO;
                }

                if (leer_inodo(entrada.ninodo, &inodo) == FALLO) // Leemos el inodo asociado a la entrada
                {
                    fprintf(stderr, RED "No ha podido leerse el inodo en mi_dir." RESET);
                    return FALLO;
                }

                if (inodo.tipo == 'd')
                {
                    // si es un direcotorio, color naranja
                    //  Metemos el nombre
                    strcat(buffer, ORANGE);
                    strcat(buffer, entrada.nombre);
                    strcat(buffer, RESET);
                    strcat(buffer, "\t"); // Agregamos una tabulación entre cada entrada
                }
                else if (inodo.tipo == 'f')
                {
                    // color azul claro
                    strcat(buffer, LBLUE);
                    strcat(buffer, entrada.nombre);
                    strcat(buffer, RESET);
                    strcat(buffer, "\t"); // Agregamos una tabulación entre cada entrada
                }
                else
                {
                    // sin color
                    strcat(buffer, entrada.nombre);
                    strcat(buffer, "\t"); // Agregamos una tabulación entre cada entrada
                }
            }
            return numEntradas;
        }

        else if (flag == 1) // CASO -l
        {

            // Si la ruta pasada corresponde a un fichero -> tan solo leemos este fichero
            if (tipo == 'f')
            {
                memset(entrada.nombre, 0, sizeof(entrada.nombre)); // Inicializamos el buffer de lectura poniendo zeros

                if (mi_read_f(p_inodo_dir, &entrada, p_entrada, sizeof(struct entrada)) == FALLO) // Leemos el fichero correspondiente
                {
                    fprintf(stderr, RED "No ha podido leerse el fichero en mi_dir." RESET);
                    return FALLO;
                }

                if (leer_inodo(entrada.ninodo, &inodo) == FALLO) // Leemos el inodo asociado a la entrada
                {
                    fprintf(stderr, RED "No ha podido leerse el inodo en mi_dir." RESET);
                    return FALLO;
                }

                // metemos el tipo
                strcat(buffer, "f\t");
                // Metemos los permisos//
                // Permisos de lectura
                if (inodo.permisos & 4)
                {
                    strcat(buffer, "r");
                }
                else
                {
                    strcat(buffer, "-");
                }
                // Permisos de escritura
                if (inodo.permisos & 2)
                {
                    strcat(buffer, "w");
                }
                else
                {
                    strcat(buffer, "-");
                }
                // Permisos de ejecución
                if (inodo.permisos & 1)
                {
                    strcat(buffer, "x\t");
                }
                else
                {
                    strcat(buffer, "-\t");
                }

                // Metemos la fecha y hora
                struct tm *tm;
                tm = localtime(&inodo.mtime);
                sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
                strcat(buffer, tmp);
                strcat(buffer, "\t");

                // Metemos el tamaño
                char size_str[20];                                               // Espacio suficiente para almacenar el tamaño convertido a cadena
                snprintf(size_str, sizeof(size_str), "%u", inodo.tamEnBytesLog); // Convertimos el tamaño a una cadena
                strcat(buffer, size_str);                                        // Concatenamos la cadena al buffer
                strcat(buffer, "\t");

                // fichero = color azul claro
                strcat(buffer, "\t"); // Agregamos un separador entre los datos y el nombre
                strcat(buffer, LBLUE);
                strcat(buffer, entrada.nombre);
                strcat(buffer, RESET);
                strcat(buffer, "\n"); // Agregamos un salto de línea al final de cada entrada

                return numEntradas;
            }

            // La ruta corresponde a un directorio -> leemos todas sus entradas
            for (int idx = 0; idx < numEntradas; idx++) // Recorremos todas las entradas
            {
                memset(entrada.nombre, 0, sizeof(entrada.nombre)); // Inicializamos el buffer de lectura poniendo zeros

                if (mi_read_f(posInodoRaiz, &entrada, idx * sizeof(struct entrada), sizeof(struct entrada)) == FALLO) // Leemos el fichero correspondiente
                {
                    fprintf(stderr, RED "No ha podido leerse el fichero en mi_dir." RESET);
                    return FALLO;
                }

                if (leer_inodo(entrada.ninodo, &inodo) == FALLO) // Leemos el inodo asociado a la entrada
                {
                    fprintf(stderr, RED "No ha podido leerse el inodo en mi_dir." RESET);
                    return FALLO;
                }

                // Metemos el tipo
                if (inodo.tipo == 'l')
                {
                    strcat(buffer, "l\t");
                }
                else if (inodo.tipo == 'd')
                {
                    strcat(buffer, "d\t");
                }
                else
                {
                    strcat(buffer, "f\t");
                }

                // Metemos los permisos//

                // Permisos de lectura
                if (inodo.permisos & 4)
                {
                    strcat(buffer, "r");
                }
                else
                {
                    strcat(buffer, "-");
                }
                // Permisos de escritura
                if (inodo.permisos & 2)
                {
                    strcat(buffer, "w");
                }
                else
                {
                    strcat(buffer, "-");
                }
                // Permisos de ejecución
                if (inodo.permisos & 1)
                {
                    strcat(buffer, "x\t");
                }
                else
                {
                    strcat(buffer, "-\t");
                }

                // Metemos la fecha y hora
                struct tm *tm;
                tm = localtime(&inodo.mtime);
                sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
                strcat(buffer, tmp);
                strcat(buffer, "\t");

                // Metemos el tamaño
                char size_str[20];                                               // Espacio suficiente para almacenar el tamaño convertido a cadena
                snprintf(size_str, sizeof(size_str), "%u", inodo.tamEnBytesLog); // Convertimos el tamaño a una cadena
                strcat(buffer, size_str);                                        // Concatenamos la cadena al buffer
                strcat(buffer, "\t");

                // Metemos el nombre
                if (inodo.tipo == 'd')
                {
                    // si es un direcotorio, color naranja
                    strcat(buffer, "\t"); // Agregamos un separador entre los datos y el nombre
                    strcat(buffer, ORANGE);
                    strcat(buffer, entrada.nombre);
                    strcat(buffer, RESET);
                    strcat(buffer, "\n"); // Agregamos un salto de línea al final de cada entrada
                }
                else if (inodo.tipo == 'f')
                {
                    // fichero = color azul claro
                    strcat(buffer, "\t"); // Agregamos un separador entre los datos y el nombre
                    strcat(buffer, LBLUE);
                    strcat(buffer, entrada.nombre);
                    strcat(buffer, RESET);
                    strcat(buffer, "\n"); // Agregamos un salto de línea al final de cada entrada
                }
                else
                {
                    // libre = sin color
                    strcat(buffer, "\t"); // Agregamos un separador entre los datos y el nombre
                    strcat(buffer, entrada.nombre);
                    strcat(buffer, "\n"); // Agregamos un salto de línea al final de cada entrada
                }
            }

            return numEntradas;
        }
        else // Valor de extra incorrecto
        {
            fprintf(stderr, RED "Error al llamar a mi_dir, el bit extra debe ser 0|1." RESET);
            return FALLO;
        }
    }
}

/* Asigna unos permisos a un fichero/directorio
 * camino: nos indica la ruta hasta el fichero/directorio
 * permisos: los permisos que asignamos al fichero/directorio
 */
int mi_chmod(const char *camino, unsigned char permisos)
{
    // Declaramos los punteros
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    unsigned int p_inodo_dir = 0;

    int posible_error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, permisos);

    if (posible_error < 0)
    {
        mostrar_error_buscar_entrada(posible_error);
        return FALLO;
    }
    else
    {
        if (mi_chmod_f(p_inodo, permisos) == FALLO)
        {
            fprintf(stderr, RED "Error en mi_chmod al llamar a mi_chmod_f" RESET);
            return FALLO;
        }
        return EXITO;
    }
}

/* Muestra la información del inodo de un fichero/directorio
 * camino: nos indica la ruta hasta el fichero/directorio
 * permisos: los permisos que asignamos al fichero/directorio
 */
int mi_stat(const char *camino, struct STAT *p_stat)
{
    // Declaramos los punteros
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    unsigned int p_inodo_dir = 0;

    int posible_error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, p_stat->permisos);

    if (posible_error == FALLO)
    {
        mostrar_error_buscar_entrada(posible_error);
        return FALLO;
    }
    else
    {
        mi_stat_f(p_inodo, p_stat);

        printf(BLUE "Nº de inodo %d\n" RESET, p_inodo);

        return EXITO;
    }
}

// NIVEL 9
// Structs UltimaEntrada para guardar cuál es la última posición que hemos o leído o escrito para ahorrar búsquedas de entradas (caché)
static struct UltimaEntrada UltimaEntradaEscritura;
static struct UltimaEntrada UltimaEntradaLectura;

/* Escribimos contenido en un fichero
 * camino: nos indica la ruta hasta el fichero/directorio
 * buf: el buffer que contiene lo que se escribirá
 * offset: posición de escritura inicial con respecto al inodo en bytes lógicos
 * nbytes: tamaño en bytes de buf
 */
int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes)
{
    // Declaramos los punteros
    unsigned int p_entrada = 0;
    unsigned int p_inodo = 0;
    unsigned int p_inodo_dir = 0;

    int bytesEscritos = 0; // Bytes escritos totales

    int posible_error = 0;

    if (strcmp(UltimaEntradaEscritura.camino, camino) == 0) // Si es la última entrada buscada -> no volvemos a buscarla
    {
        p_inodo = UltimaEntradaEscritura.p_inodo;
        // fprintf(stderr, BLUE "[mi_write() → Utilizamos la caché de escritura en vez de llamar a buscar_entrada()]\n" RESET);
    }
    else
    { // No es la última entrada buscada -> buscamos la entrada

        posible_error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 6);

        if (posible_error < 0) // Si hay un error, lo enseñados
        {
            mostrar_error_buscar_entrada(posible_error);
            return FALLO;
        }
        else
        { // Si ha ido todo bien -> el que acabamos de buscar es el último buscado
            UltimaEntradaEscritura.p_inodo = p_inodo;
            strcpy(UltimaEntradaEscritura.camino, camino);
            // fprintf(stderr, RED "[mi_write() → Actualizamos la caché de escritura]\n" RESET);
        }
    }

    bytesEscritos = mi_write_f(p_inodo, buf, offset, nbytes);

    return bytesEscritos;
}

/* Leemos contenido de un fichero
 * camino: nos indica la ruta hasta el fichero/directorio
 * buf: el buffer en el que se guardará lo leído (debe estar inicializado a 0's)
 * offset: posición de lectura inicial con respecto al inodo en bytes lógicos
 * nbytes: nº de bytes a leer
 */
int mi_read(const char *camino, void *buf, unsigned int offset, unsigned int nbytes)
{
    // Declaramos los punteros
    unsigned int p_entrada = 0;
    unsigned int p_inodo = 0;
    unsigned int p_inodo_dir = 0;

    int posible_error = 0;

    int bytesLeidos = 0;

    if (strcmp(UltimaEntradaLectura.camino, camino) == 0) // Si es la última entrada buscada -> no volvemos a buscarla
    {
        // fprintf(stderr, BLUE "\n[mi_read() → Utilizamos la caché de lectura en vez de llamar a buscar_entrada()]\n" RESET);
        p_inodo = UltimaEntradaLectura.p_inodo;
    }
    else
    { // Si no es la última entrada buscada -> la buscamos

        posible_error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 0);

        if (posible_error < 0)
        {
            mostrar_error_buscar_entrada(posible_error);
            return FALLO;
        }
        else // Si va todo bien, guardamos la entrada buscada como última entrada buscada
        {
            UltimaEntradaLectura.p_inodo = p_inodo;
            strcpy(UltimaEntradaLectura.camino, camino);

            // fprintf(stderr, RED "[mi_read() → Actualizamos la caché de lectura]\n" RESET);
        }
    }

    bytesLeidos = mi_read_f(p_inodo, buf, offset, nbytes);

    if (bytesLeidos == FALLO)
    {
        fprintf(stderr, RED "Error en mi_read al escribir con mi_read_f en directorios.c");
        return FALLO;
    }

    return bytesLeidos;
}

// NIVEL 10

/* Crea el enlace de una entrada de directorio camino2 al inodo especificado por otra entrada de directorio camino1
 * camino1: entrada de directorio de destino
 * camino2: entrada de directorio de origen
 */
int mi_link(const char *camino1, const char *camino2)
{

    mi_waitSem();

    // Declaramos los structs que vamos a usar
    struct entrada entrada;
    struct inodo inodo;

    // Declaramos los punteros para la búsqueda del camino 1
    unsigned int p_entrada1 = 0;
    unsigned int p_inodo1 = 0;
    unsigned int p_inodo_dir1 = 0;

    // Declaramos los punteros para la búsqueda del camino 2
    unsigned int p_entrada2 = 0;
    unsigned int p_inodo2 = 0;
    unsigned int p_inodo_dir2 = 0;

    int posible_error = 0;

    // Camino 1
    posible_error = buscar_entrada(camino1, &p_inodo_dir1, &p_inodo1, &p_entrada1, 0, 4); // Comprobamos si existe(reservar = 0) y comprobamos que tiene permisos de lectura(permisos 4).

    if (posible_error < 0) // Si el fichero 1 no existe -> mostramos error
    {
        mostrar_error_buscar_entrada(posible_error);
        mi_signalSem();
        return FALLO;
    }

    // Camino 2
    posible_error = buscar_entrada(camino2, &p_inodo_dir2, &p_inodo2, &p_entrada2, 1, 6);

    if (posible_error < 0) // Si ya existe -> mostramos error
    {
        mostrar_error_buscar_entrada(posible_error);
        mi_signalSem();
        return FALLO;
    }

    // Leemos la entrada correspondiente a camino2
    posible_error = mi_read_f(p_inodo_dir2, &entrada, p_entrada2 * sizeof(entrada), sizeof(entrada));
    if (posible_error < 0)
    {
        fprintf(stderr, RED "Error en mi_link al ejecutar mi_read_f\n" RESET);
        mi_signalSem();
        return FALLO;
    }

    // Asociamos a la entrada el mismo inodo que el asociado a p_inodo1
    entrada.ninodo = p_inodo1;

    // Escribimos la entrada modificada en p_inodo_dir2
    mi_write_f(p_inodo_dir2, &entrada, p_entrada2 * sizeof(entrada), sizeof(entrada));

    if (posible_error < 0)
    {
        fprintf(stderr, RED "Error en mi_link al ejecutar mi_write_f\n" RESET);
        mi_signalSem();
        return FALLO;
    }

    // Liberamos el inodo asociado a la entrada creada
    if (liberar_inodo(p_inodo2) == FALLO)
    {
        fprintf(stderr, RED "Error en mi_link al liberar el inodo" RESET);
        mi_signalSem();
        return FALLO;
    }

    // Leemos el inodo asociado a camino1 para actualizar el valor de sus atributos
    if (leer_inodo(p_inodo1, &inodo) == FALLO)
    {
        fprintf(stderr, RED "Error en mi_link al leer el inodo\n" RESET);
        mi_signalSem();
        return FALLO;
    }

    inodo.nlinks++;           //+1 enlace
    inodo.ctime = time(NULL); // Actualizamos el ctime

    // Guardamos los cambios realizados en sus atributos
    if (escribir_inodo(p_inodo1, &inodo) == FALLO)
    {
        fprintf(stderr, RED "Error en mi_link al escribir el inodo\n" RESET);
        mi_signalSem();
        return FALLO;
    }

    mi_signalSem();
    return EXITO;
}

/** Borra la entrada de directorio especificada. Si es el último enlace, borra el propio fichero
 * camino: camino donde encontrar la entrada de directorio a borrar
 */
int mi_unlink(const char *camino)
{

    mi_waitSem();

    // Declaramos los punteros
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    unsigned int p_inodo_dir = 0;

    int posible_error = 0;

    // Inodo del camino
    struct inodo inodo;

    // Inodo asociado al directorio con la entrada a borrar
    struct inodo inodo_dir;

    struct entrada entrada;

    // Comprobamos que la entrada camino exista y obtenemos su nº de entrada (p_entrada)
    posible_error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 6);
    if (posible_error < 0)
    {
        mostrar_error_buscar_entrada(posible_error);
        mi_signalSem();
        return FALLO;
    }

    // Leemos el inodo correspondiente al camino
    if (leer_inodo(p_inodo, &inodo) == FALLO)
    {
        fprintf(stderr, RED "Error en mi_unlink al leer inodo\n" RESET);
        mi_signalSem();
        return FALLO;
    }

    // Si se trata de un directorio y no está vacío (inodo.tamEnBytesLog > 0) entonces no se puede borrar y salimos de la función.
    if (inodo.tipo == 'd' && inodo.tamEnBytesLog > 0)
    {
        fprintf(stderr, RED "Error: El directorio %s no está vacío\n" RESET, camino);
        mi_signalSem();
        return FALLO;
    }

    else // Está vacío -> lo borramos
    {
        // Leemos el inodo asociado al directorio que contiene la entrada que queremos eliminar (p_inodo_dir)
        if (leer_inodo(p_inodo_dir, &inodo_dir) == FALLO)
        {
            fprintf(stderr, RED "Error en mi_unlink ya que no se puede leer inodo\n" RESET);
            mi_signalSem();
            return FALLO;
        }

        // Obtenemos el nº de entradas que tiene
        int n_entradas = inodo_dir.tamEnBytesLog / sizeof(struct entrada);

        if (p_entrada == (n_entradas - 1))
        {
            // Si la entrada a eliminar es la última (p_entrada ==nº entradas - 1) basta con truncar el inodo a su tamaño menos el tamaño de una entrada.
            // Se hace al salir del if
        }
        else
        { // No es la última entrada -> tenemos que leer la última y escribirla en la posición de la entrada que queremos eliminar

            // Leemos la última entrada
            if (mi_read_f(p_inodo_dir, &entrada, inodo_dir.tamEnBytesLog - sizeof(struct entrada), sizeof(struct entrada)) == FALLO)
            {
                fprintf(stderr, RED "Error en mi_unlink al leer la última entrada" RESET);
                mi_signalSem();
                return FALLO;
            }

            // La escribimos en la posición que queremos eliminar
            if (mi_write_f(p_inodo_dir, &entrada, p_entrada * sizeof(struct entrada), sizeof(struct entrada)) == FALLO)
            {
                fprintf(stderr, RED "Error en mi_unlink al escribir en la última posicion" RESET);
                mi_signalSem();
                return FALLO;
            }

            // Ahora ya podemos truncar
        }

        // Truncamos el inodo
        if (mi_truncar_f(p_inodo_dir, inodo_dir.tamEnBytesLog - sizeof(struct entrada)) == FALLO)
        {
            fprintf(stderr, RED "Error en mi_unlink al truncar el inodo" RESET);
            mi_signalSem();
            return FALLO;
        }

        // Leemos el inodo asociado a la entrada eliminada para decrementar el nº de enlaces.
        if (leer_inodo(p_inodo, &inodo_dir) == FALLO)
        {
            fprintf(stderr, RED "Error en mi_unlink al leer el inodo\n" RESET);
            mi_signalSem();
            return FALLO;
        }

        // Decrementamos el n° de enlaces
        inodo_dir.nlinks--;

        // Si no quedan enlaces entonces liberamos el inodo
        if (inodo_dir.nlinks == 0)
        {
            if (liberar_inodo(p_inodo) == FALLO)
            {
                fprintf(stderr, RED "Error en mi_unlink al liberar el inodo" RESET);
                mi_signalSem();
                return FALLO;
            }
        }
        else // En caso contrario actualizamos su ctime y escribimos el inodo.
        {
            // actualizamos su ctime
            inodo_dir.ctime = time(NULL);
            if (escribir_inodo(p_inodo, &inodo_dir) == FALLO)
            {
                fprintf(stderr, RED "Error en mi_unlink al escribir el inodo" RESET);
                mi_signalSem();
                return FALLO;
            }
        }
    }
    mi_signalSem();
    return EXITO;
}
