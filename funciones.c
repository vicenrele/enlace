/******************************************************************************
 * FICHERO: funciones.c                                                       *
 ******************************************************************************
 *  - Funciones para manipular las tramas.                                    *
 ******************************************************************************/
 
#include <stdio.h>
#include "funciones.h"

// Función que muestra por pantalla una trama recibida por referencia
void mostrarTrama(char *trama, int tam)
{
	int i;
	
	fprintf(stdout, "    Composicion Trama ......: "); 
    fflush(stdout);
    
	for(i = 0; i < tam; i++){
		switch(trama[i]){
			case SYN:
				fprintf(stdout, "SYN ");
                fflush(stdout);
				break;
			case SOH:
				fprintf(stdout, "SOH ");
                fflush(stdout);
				break;
			case STX:
				fprintf(stdout, "STX ");
                fflush(stdout);
				break;
			case ETX:
				fprintf(stdout, "ETX ");
                fflush(stdout);
				break;
			case EOT:
				fprintf(stdout, "EOT ");
                fflush(stdout);
				break;
			case ENQ:
				fprintf(stdout, "ENQ ");
                fflush(stdout);
				break;
			case ACK:
				fprintf(stdout, "ACK ");
                fflush(stdout);
				break;
			case DLE:
				fprintf(stdout, "DLE ");
                fflush(stdout);
				break;
			case NAK:
				fprintf(stdout, "NAK ");
                fflush(stdout);
				break;
			default:
				fprintf(stdout, "%c ", trama[i]);
                fflush(stdout);
				break;
        }	
	} 
}

// Función que genera una trama de solicitud de conexión
void solicitarConexion(char *trama, int *tam){
	trama[0]=DLE;
	trama[1]=SYN;
	trama[2]=DLE;
	trama[3]=SYN;
	trama[4]=DLE;
	trama[5]=ENQ;
	
	*tam = 6;
}

// Función que genera una trama de conexión aceptada
void conexionAceptada(char *trama, int *tam){
	trama[0]=DLE;
	trama[1]=SYN;
	trama[2]=DLE;
	trama[3]=SYN;
	trama[4]=DLE;
	trama[5]=ACK;
	
	*tam = 6;
}

// Función que genera una trama de desconexión
void desconexion(char *trama, int *tam){
	trama[0]=DLE;
	trama[1]=SYN;
	trama[2]=DLE;
	trama[3]=SYN;
	trama[4]=DLE;
	trama[5]=EOT;
	
	*tam = 6;
		
}

// Función que genera una trama de confirmación
void confirmacionTrama(char *trama, int *tam){
	trama[0]=DLE;
	trama[1]=SYN;
	trama[2]=DLE;
	trama[3]=SYN;
	trama[4]=DLE;
	trama[5]=ACK;
	trama[6]='0';    
		
	*tam = 7;
		
}

// Función que genera una trama de datos (pasados por referencia)
void tramaDatos(char *trama, int *tam, char *datos, int *tamDatos, char nSec){
	char tramaReducida[1500];
	int checksum;
	int i,j,d;
	char hibyte, lobyte;

	// Montamos la trama de datos sin los caracteres de sincronismo
	tramaReducida[0]=SOH;
	tramaReducida[1]=nSec;
	tramaReducida[2]=DLE;
	tramaReducida[3]=STX;
	
	for (i=4,j=0; j<*tamDatos; i++,j++)
	{
	    tramaReducida[i] = datos[j];
		if (datos[j] == DLE){
			tramaReducida[i+1] = DLE;
			i++;
		}
	}
	*tam=i+2;
	tramaReducida[*tam-2] = DLE;
	tramaReducida[*tam-1] = ETX;
	
	
	// Calculamos el Checksum
	checksum = icrc(0,(unsigned char *)tramaReducida, *tam-1, 255, -1);

	// Calculamos los caracteres CRC
	*tam = *tam+2;
	tramaReducida[*tam-2] = (char)HIBYTE(checksum);    // CCEH
	tramaReducida[*tam-1] = (char)LOBYTE(checksum);    // CCEL

    // Generamos la trama de datos completa (con los CRC)	
	trama[0]=DLE;
	trama[1]=SYN;
	trama[2]=DLE;
	trama[3]=SYN;
	trama[4]=DLE;
	
	for (d=0; d < *tam; d++)
	{
	 	trama[d+5] = tramaReducida[d];
	}
	*tam = *tam+5;

}

// Función que identifica el tipo de una trama o si es una trama no valida
int analizarTrama (char *trama, int *tamTrama, char *datos, int *tamDatos) 
{
    int t; 
    int d = 0;
    //mostrarTrama (trama, *tamTrama);
    printf("\n");
    // Trama fallida: retorna 1
    if (*tamTrama > 1500)
        return 1;
    if (*tamTrama > 0 && trama[0] != DLE)
		return 1;   
	if (*tamTrama > 1 && trama[1] != SYN)
	    return 1;
	if (*tamTrama > 2 && trama[2] != DLE)
	    return 1;
	if (*tamTrama > 3 && trama[3] != SYN)
	    return 1;
	if (*tamTrama > 4 && trama[4] != DLE)
	    return 1;
    if (*tamTrama > 5 && trama[5] != ENQ && trama[5] != ACK && trama[5] != NAK
	    && trama[5] != SOH && trama[5] != EOT)
	    return 1;
	
    // Trama de solicitud de conexión: retorna 2
    if (*tamTrama == 6 && trama[5] == ENQ)    
	    return 2;   
        
    // Trama de conexión rechazada: retorna 3   
   	if (*tamTrama == 6 && trama[5] == NAK)
	    return 3;  
        
    // Trama de desconexión: retorna 4     
	if (*tamTrama == 6 && trama[5] == EOT)
	    return 4;    
	
	// Trama de conexión aceptada: retorna 5
    if (*tamTrama == 6 && trama[5] == ACK)
		    return 5;    
		    
	// Trama de confirmación de trama: retorna 6	    
	if (*tamTrama == 7 && trama[5] == ACK && (trama[6] == '0' || trama[6] == '1'))
	    return 6;    
	    
	// Trama de datos: retorna 7 y almacena lo que es dato en la cadena "datos" 
	// pasada por referencia. En caso contrario retorna 1 (Trama fallida)
	if (*tamTrama > 11 && trama[5] == SOH && (trama[6] == '0' || trama[6] == '1') 
	    && trama[7] == DLE && trama[8] == STX) 
										
	    for (t = 9; t < *tamTrama; t++) {  
            if (trama[t] != DLE) {  
                datos[d] = trama[t];
                d++;       
            }
            else {
                 if (trama[t + 1] == DLE) {
                     datos[d] = trama[t];  
                     d++; t++;
                 }
                 if (trama[t + 1] == ETX){
					 *tamDatos = d;
                     return 7;
                 }             
            }  
        }   
    return 1;  
}

// Comprueba los códigos de control de redundancia de las tramas de datos
int validarCrc (char *trama, int *tam)
{
    char tramaReducida[1500];
	int checksum;
	int d, tamRed;
	char cceh, ccel;
	char hibyte, lobyte;
	
	// Almacenamos los CRC recibidos
	cceh = (char)trama[*tam-2];
	ccel = (char)trama[*tam-1];
	
	tamRed = *tam-7;
	
	// Montamos la trama de datos sin los caracteres de sincronismo
	for (d=0; d<tamRed ; d++)
	{
	    tramaReducida[d] = trama[d+5];
	}
	
    // Calculamos el Checksum
	checksum = icrc(0,(unsigned char *)tramaReducida, tamRed-1, 255, -1);
	hibyte = (char) HIBYTE(checksum);
	lobyte = (char)LOBYTE(checksum);
	
    // Comparamos los valores de los caracteres CRC
	if (cceh == hibyte && ccel == lobyte)
	    return 0;    // Éxito
    else
        return 1;    
}
             
// Función que identifica y muestra el tipo de una trama
void tipoTrama(int valorRet)
{
    switch (valorRet) 
    {
	 	
        case 1:
		    fprintf(stdout, "    Trama Recibida .........: TRAMA FALLIDA\n");
		    fflush(stdout);
            break;
		case 2:
		    fprintf(stdout, "    Trama Recibida .........: SOLICITUD DE CONEXION\n");
		    fflush(stdout);
            break;
		case 3:
		    fprintf(stdout, "    Trama Recibida .........: CONEXION RECHAZADA\n");
		    fflush(stdout);
            break;
		case 4:
		    fprintf(stdout, "    Trama Recibida .........: DESCONEXION\n");
		    fflush(stdout);
            break;
		case 5:
		    fprintf(stdout, "    Trama Recibida .........: CONEXION ACEPTADA\n");
		    fflush(stdout);
            break;
		case 6:
		    fprintf(stdout, "    Trama Recibida .........: CONFIRMACION DE TRAMA\n");
		    fflush(stdout);
            break;
		case 7:
		    fprintf(stdout, "    Trama Recibida .........: TRAMA DE DATOS\n");
		    fflush(stdout);
            break;
		default:
		    fprintf(stdout, "    Trama Recibida .........: TRAMA FALLIDA\n");
		    fflush(stdout);
            break;
	}
}
	
