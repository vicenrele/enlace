/******************************************************************************
 * FICHERO: funciones.h                                                       *
 ******************************************************************************
 *  - Prototipos de las funciones de manejo de trama y las del servicio de    *
 *    enlace de conexión.                                                     *
 *  - Definición de las constantes literales para los caracteres especiales   *
 *    de las tramas.                                                          *
 *  - Definición de la función de comprobación de errores.                    *
 ******************************************************************************/

#include "fisico.h"

// Definición de los caracteres especiales
#define SYN 22
#define SOH 1
#define STX 2
#define ETX 3
#define EOT 4
#define ENQ 5
#define ACK 6
#define DLE 16
#define NAK 21

// Definición de Perror (llamadas al sistema)
#define PERROR(a) \
	{             \
	     LPVOID lpMsg;                     \
		 FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | \
		               FORMAT_MESSAGE_FROM_SYSTEM |     \
					   FORMAT_MESSAGE_IGNORE_INSERTS, NULL, \
					   GetLastError(), \
					   MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT), \
					   (LPTSTR) &lpMsg, 0, NULL);  \
		 fprintf(stderr,"%s:(%d)%s\n",a,GetLastError(),lpMsg); \
         LocalFree(lpMsg); \
     }
    
// Prototipo de las funciones de manejo de tramas
void mostrarTrama(char *trama, int tam);
void solicitarConexion(char *trama, int *tam);
void conexionAceptada(char *trama, int *tam);
void conexionRechazada(char *trama, int *tam);
void desconexion(char *trama, int *tam);
int  analizarTrama(char *trama, int *tam, char *cadena, int *tamCadena);
void tramaDatos(char *trama, int *tam, char *datos, int *tamDatos, char nSec);
int  cargarBiblioteca(char lado);
void tipoTrama(int valorRet);

// Prototipo de las funciones del nivel de aplicación
int ladoIzquierdo(void);
int ladoDerecho(void);

// Prototipo de las funciones de comprobación de redundancia cíclica
unsigned short icrc1(unsigned short crc, unsigned char onech);
unsigned short icrc(unsigned short crc, unsigned char *bufptr,
					unsigned long len, short jinit, int jrev);

