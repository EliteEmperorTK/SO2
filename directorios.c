#include "directorios.h"

// NIVEL 7

void mostrar_error_buscar_entrada(int error)
{
    switch (error)
    {
    case -2:
        perror(RED "Error: Camino incorrecto.\n" RESET);
        break;
    case -3:
        perror(RED "Error: Permiso denegado de lectura.\n" RESET);
        break;
    case -4:
        perror(RED "Error: No existe el archivo o el directorio.\n" RESET);
        break;
    case -5:
        perror(RED "Error: No existe algún directorio intermedio.\n" RESET);
        break;
    case -6:
        perror(RED "Error: Permiso denegado de escritura.\n" RESET);
        break;
    case -7:
        perror(RED "Error: El archivo ya existe.\n" RESET);
        break;
    case -8:
        perror(RED "Error: No es un directorio.\n" RESET);
        break;
    }
}

// Modifica inicial y final con el trozo de directorio correspondiente del camino.
int extraer_camino(const char *camino, char *inicial, char *final, char *tipo)
{

    if (camino == NULL || *camino != '/')
    { // Comprobamos que el formato sea correcto
        mostrar_error_buscar_entrada(ERROR_CAMINO_INCORRECTO);
        return ERROR_CAMINO_INCORRECTO;
    }

    // Encontrar la primera '/' después de la primera posición
    const char *segunda_barra = strchr(camino + 1, '/'); // Si no la encuentra, devuelve NULL

    if (segunda_barra == NULL)
    {
        // Si no hay segunda '/', el camino solo contiene el nombre de un fichero
        strcpy(inicial, camino + 1); // Copiar el nombre del fichero a *inicial
        *tipo = 'f';

        strcpy(final, ""); // No hay más partes del camino
    }
    else
    {
        // Si hay segunda '/', dividir el camino en *inicial y *final
        strncpy(inicial, camino + 1, segunda_barra - (camino + 1)); // Copiar *inicial desde después de la primera barra

        strcpy(final, segunda_barra); // Copiamos en *final desde la segunda barra (inclusive) hasta el \0

        // Determinar si el nombre en inicial representa un directorio o un fichero
        if (final[strlen(final) - 1] == '/')
        {
            *tipo = 'd'; // Directorio si el último carácter es '/'
        }
        else
        {
            *tipo = 'f'; // Sinó es un fichero
        }
    }

    return EXITO; // Éxito
}

int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos)
{
    struct entrada entrada;
    struct inodo inodo_dir;
    char inicial[sizeof(entrada.nombre)];
    char final[strlen(camino_parcial)];
    char tipo;
    int cant_entradas_inodo, num_entrada_inodo;

    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO)
    {
        perror(RED "Error al leer el superbloque.\n" RESET);
        return FALLO;
    }

    if (strcmp(camino_parcial, "/") == 0) // camino_parcial es “/”
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
    fprintf(stderr, "[buscar_entrada()→ inicial: %s, final: %s, reservar: %d]\n", inicial, final, reservar);
    // buscamos la entrada cuyo nombre se encuentra en inicial
    if (leer_inodo(*p_inodo_dir, &inodo_dir) == FALLO)
    {
        perror(RED "Error al leer el inodo en buscar_entrada" RESET);
        return FALLO;
    }

    if ((inodo_dir.permisos & 4) != 4)
    { // Miramos si el inodo tiene permisos de lectura
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
        if (mi_read_f(*p_inodo_dir, &entrada, 0, sizeof(struct entrada)) < 0)
        { // Leer entrada
            perror(RED "Error al leer entrada en buscar_entrada" RESET);
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
                perror(RED "Error al leer entrada en buscar_entrada" RESET);
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
                    if (strcmp(final, "/") == 0)
                    {
                        // reservar un inodo como directorio y asignarlo a la entrada
                        entrada.ninodo = reservar_inodo('d', permisos);
                        fprintf(stderr, "[buscar_entrada()→ reservado inodo %d tipo %c con permisos %d para %s]\n", entrada.ninodo, tipo, permisos, inicial);
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
                    fprintf(stderr, "[buscar_entrada()→ reservado inodo %d tipo %c con permisos %d para %s]\n", entrada.ninodo, tipo, permisos, inicial);
                }
                // escribir la entrada en el directorio padre
                if (mi_write_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada)) == FALLO)
                {                             // error de escritura
                    if (entrada.ninodo != -1) // si habiamos reservado un inodo
                    {
                        // liberar el inodo
                        liberar_inodo(entrada.ninodo);
                        fprintf(stderr, "[buscar_entrada()-> liberado inodo %i, reservado a %s]\n", num_entrada_inodo, inicial);
                    }
                    else
                    {
                        return FALLO;
                    }
                }
                fprintf(stderr, "[buscar_entrada()-> creada entrada: %s, %d]\n", entrada.nombre, entrada.ninodo);
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
int mi_creat(const char *camino, unsigned char permisos)
{
    int posible_error;
    unsigned int inodoDir = 0;
    unsigned int inodo = 0;
    unsigned int entrada = 0;

    posible_error = buscar_entrada(camino, &inodoDir, &inodo, &entrada, 1, permisos); // Llamamos a buscar_entrada con reservar = 1

    if (posible_error == FALLO) // Si hay un error, lo mostramos por pantalla
    {
        mostrar_error_buscar_entrada(posible_error);
        return FALLO;
    }

    return EXITO;
}

// Pone el contenido del directorio(camino) en un buffer de memoria(buffer)
int mi_dir(const char *camino, char *buffer)
{
    struct superbloque SB;
    struct inodo inodo;

    struct entrada entrada;

    char tmp[100];
    
    int numEntradas;    // Cantidad de entradas del inodo
    int posibleError;

    if (bread(posSB, &SB) == FALLO)
    {
        perror(RED "Error al leer el superbloque en el disco.\n" RESET);
        return FALLO;
    }

    unsigned int entradas = 0;
    unsigned int inodoDir = SB.posInodoRaiz;
    unsigned int inodos = SB.posInodoRaiz;

    posibleError = buscar_entrada(camino, &inodoDir, &inodos, &entradas, 0, 0); // Buscamos la entrada

    if (posibleError < 0) // Si hay un error lo enseñamos por pantalla
    {
        mostrar_error_buscar_entrada(posibleError);
        return FALLO;
    }
    else
    {
        leer_inodo(inodos, &inodo); // Leemos el inodo
        numEntradas = inodo.tamEnBytesLog / sizeof(struct entrada);

        if (inodo.tipo != 'd') // Si no es un directorio, retornamos error
        {
            mostrar_error_buscar_entrada(ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO);
            return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO;
        }

        if ((inodo.permisos & 4) != 4) //Comprobamos que el inodo tenga permisos de lectura
        {
            mostrar_error_buscar_entrada(ERROR_PERMISO_LECTURA);
            return ERROR_PERMISO_LECTURA;
        }
       
        for (int idx = 0; idx < numEntradas; idx++) // Recorremos todas las entradas
        {
            memset(entrada.nombre, 0, sizeof(entrada.nombre)); // Inicializamos el buffer de lectura poniendo zeros

            if (mi_read_f(inodos, &entrada, idx * sizeof(struct entrada), sizeof(struct entrada)) == FALLO) // Leemos el fichero correspondiente
            {
                perror(RED "No ha podido leerse el fichero en mi_dir." RESET);
                return FALLO;
            }


            if (leer_inodo(entrada.ninodo, &inodo) == FALLO) // Leemos el inodo asociado a la entrada
            {
                perror(RED "No ha podido leerse el inodo en mi_dir." RESET);
                return FALLO;
            }


            // Permisos
            if (inodo.permisos & 4) // Permisos de lectura
            {
                strcat(buffer, "r");
            }
            else
            {
                strcat(buffer, "-");
            }


            if (inodo.permisos & 2) // Permisos de escritura
            {
                strcat(buffer, "w");
            }
            else
            {
                strcat(buffer, "-");
            }


            if (inodo.permisos & 1) // Permisos de ejecución
            {
                strcat(buffer, "x");
            }
            else
            {
                strcat(buffer, "-");
            }

            // Modificamos el valor del tiempo
            struct tm *tm;
            tm = localtime(&inodo.mtime);
            sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
            strcat(buffer, tmp);
        }
        return numEntradas;
    }
}

int mi_chmod(const char *camino, unsigned char permisos)
{
    unsigned int inodo = 0;
    unsigned int entrada = 0;
    unsigned int inodoDir = 0;

    int posible_error = buscar_entrada(camino, &inodoDir, &inodo, &entrada, 0, permisos);

    if (posible_error == FALLO)
    {
        mostrar_error_buscar_entrada(posible_error);
        return FALLO;
    }
    else
    {
        mi_chmod_f(inodo, permisos);
        return EXITO;
    }
}

int mi_stat(const char *camino, struct STAT *p_stat)
{
    unsigned int inodo = 0;
    unsigned int entrada = 0;
    unsigned int inodoDir = 0;

    int posible_error = buscar_entrada(camino, &inodoDir, &inodo, &entrada, 0, p_stat->permisos);

    if (posible_error == FALLO)
    {
        mostrar_error_buscar_entrada(posible_error);
        return FALLO;
    }
    else
    {
        mi_stat_f(inodo, p_stat);
        return EXITO;
    }
}
