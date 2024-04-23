#include "directorios.h"

// NIVEL 7

void mostrar_error_buscar_entrada(int error)
{
    // fprintf(stderr, "Error: %d\n", error);
    switch (error)
    {
    case -2:
        fprintf(stderr, "Error: Camino incorrecto.\n");
        break;
    case -3:
        fprintf(stderr, "Error: Permiso denegado de lectura.\n");
        break;
    case -4:
        fprintf(stderr, "Error: No existe el archivo o el directorio.\n");
        break;
    case -5:
        fprintf(stderr, "Error: No existe algún directorio intermedio.\n");
        break;
    case -6:
        fprintf(stderr, "Error: Permiso denegado de escritura.\n");
        break;
    case -7:
        fprintf(stderr, "Error: El archivo ya existe.\n");
        break;
    case -8:
        fprintf(stderr, "Error: No es un directorio.\n");
        break;
    }
}

int extraer_camino(const char *camino, char *inicial, char *final, char *tipo)
{

    if (camino == NULL || *camino != '/')
    { // Comprobamos que el formato sea correcto
        //////////mensaje de error
        return FALLO;
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
    char final[sizeof(strlen(camino_parcial))];
    char tipo;
    int cant_entradas_inodo, num_entrada_inodo;

    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO)
    {
        fprintf(stderr, "Error al leer el superbloque.\n");
        return FALLO;
    }

    if (strcmp(camino_parcial, "/") == 0)
    {                               // camino_parcial es “/”
        *p_inodo = SB.posInodoRaiz; // nuestra raiz siempre estará asociada al inodo 0
        *p_entrada = 0;
        return EXITO;
    }

    extraer_camino(camino_parcial, inicial, final, &tipo);

    if (extraer_camino(camino_parcial, inicial, final, &tipo) == FALLO)
    {
        mostrar_error_buscar_entrada(ERROR_CAMINO_INCORRECTO);
        return ERROR_CAMINO_INCORRECTO;
    }

    // buscamos la entrada cuyo nombre se encuentra en inicial
    leer_inodo(*p_inodo_dir, &inodo_dir);

    if ((inodo_dir.permisos & 4) != 4)
    { // Miramos si el inodo tiene permisos de lectura
        mostrar_error_buscar_entrada(ERROR_PERMISO_LECTURA);
        return ERROR_PERMISO_LECTURA;
    }

    // inicializar el buffer de lectura con 0s
    memset(entrada.nombre, 0, sizeof(entrada.nombre)); // Inicializar buffer de lectura con 0s

    // el buffer de lectura puede ser un struct tipo entrada
    // o mejor un array de las entradas que caben en un bloque, para optimizar la lectura en RAM

    cant_entradas_inodo = inodo_dir.tamEnBytesLog / sizeof(struct entrada);
    num_entrada_inodo = 0; // nº de entrada inicial;

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
            if (mi_read_f(*p_inodo_dir, &entrada, 0, sizeof(struct entrada)) < 0)
            { // Leer entrada
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
            mostrar_error_buscar_entrada(ERROR_NO_EXISTE_ENTRADA_CONSULTA);
            return ERROR_NO_EXISTE_ENTRADA_CONSULTA;

        case 1: // modo escritura
            // Creamos la entrada en el directorio referenciado por *p_inodo_dir
            // si es fichero no permitir escritura
            if (inodo_dir.tipo == 'f')
            {
                mostrar_error_buscar_entrada(ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO);
                return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO;
            }
            // si es directorio comprobar que tiene permiso de escritura
            if ((inodo_dir.permisos & 2) != 2)
            { // No tiene permisos de escritura
                mostrar_error_buscar_entrada(ERROR_PERMISO_ESCRITURA);
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
                    }
                    else
                    {
                        mostrar_error_buscar_entrada(ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO);
                        return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
                    }
                }
                else // es un fichero
                {
                    // reservar un inodo como fichero y asignarlo a la entrada
                    entrada.ninodo = reservar_inodo('f', permisos);
                }
                // escribir la entrada en el directorio padre
                if (mi_write_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada)) == FALLO)
                {                             // error de escritura
                    if (entrada.ninodo != -1) // si habiamos reservado un inodo
                    {
                        // liberar el inodo
                        liberar_inodo(entrada.ninodo);
                    }
                    return FALLO;
                }
            }
        }
    }
    if ((strcmp(final, "") == 0) || (strcmp(final, "/") == 0))
    { // hemos llegado al final del camino

        if ((num_entrada_inodo < cant_entradas_inodo) && (reservar == 1))
        {
            // modo escritura y la entrada ya existe
            mostrar_error_buscar_entrada(ERROR_ENTRADA_YA_EXISTENTE);
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
