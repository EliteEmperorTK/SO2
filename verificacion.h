/* AUTORES:
 * Marc Nadal Sastre Gondar
 * Joaquín Esperon Solari
 * Martí Vich Gispert
 */

/* verificacion.h */
#include "simulacion.h"

struct INFORMACION
{
    int pid;
    unsigned int nEscrituras; // validadas
    struct REGISTRO PrimeraEscritura;
    struct REGISTRO UltimaEscritura;
    struct REGISTRO MenorPosicion;
    struct REGISTRO MayorPosicion;
};
