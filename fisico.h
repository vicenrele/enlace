/******************************************************************************
 * FICHERO: funciones.c                                                       *
 ******************************************************************************
 *  - Definimos los prototipos de las funciones de nivel físico.              *
 ******************************************************************************  
 * AUTORES                                                                    *
 * -------                                                                    *
 * Leandro Adeodato de Albuquerque                                            *
 * vicenrele                                                                  *
 ******************************************************************************/

#include <windows.h>

// El bloque ifdef siguiente es la manera estándar de creación de macros 
// para la exportación simple de DLL
#ifdef TUBODLL_EXPORTS
#define TUBODLL_API __declspec(dllexport)
#else
#define TUBODLL_API __declspec(dllimport)
#endif

typedef DWORD (*TIPO_F_PUEDO_ESCRIBIR)(void);
typedef DWORD (*TIPO_F_ESPERAR_ESCRIBIR)(DWORD);
typedef DWORD (*TIPO_F_ESCRIBIR)(char);
typedef DWORD (*TIPO_F_PUEDO_LEER)(void);
typedef DWORD (*TIPO_F_ESPERAR_LEER)(DWORD);
typedef DWORD (*TIPO_F_LEER)(char *);
typedef BOOL  (*TIPO_F_INICIO_PUERTO)(char, char*, char*,DWORD, void (*)(void),DWORD);
