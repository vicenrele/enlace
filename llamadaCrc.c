#ifndef _CRC_H_
#define _CRC_H_

typedef unsigned char uchar;

unsigned short icrc1(unsigned short crc, unsigned char onech);
unsigned short icrc(unsigned short crc, unsigned char *bufptr,
					unsigned long len, short jinit, int jrev);

#endif


////////////////////////////////////////////////////////////////////////////////////
// CreaTramaDatos
//
//	Construye la trama de datos con el siguiente formato
//	DLE SOH n_seq DLE STX | datos
//			datos (cont.) | DLE ETX | CRC (16bits)
//
// Ej: trama = CreaTramaDatos(info);
//
// Entrada: 
//		UnidadDatos *trama		información a viajar en la trama
// Retorno:
//		UnidadDatos *			puntero a la trama creada.
//
// Pseudocódigo:
//		1) Construye una trama de datos para la transmisión por el puerto serie.
//
///////////////////////////////////////////////////////////////////////////////////

UnidadDatos * CLLC::CreaTramaDatos(UnidadDatos *trama)
{
	UnidadDatos *temp;
	int tam = trama->longitud;
	int contadorDLE = 0;
	int i,j;
	int longCab = 5; // DLE SOH n_seq DLE STX
	char inicio[2]={0x10, 0x01}; // DLE SOH
	unsigned short checksum = 0;
	

	for ( i=0 ; i<tam ; i++)
		if ( trama->datos[i] == 0x10 ) 
			contadorDLE++;
	
	temp = (UnidadDatos *)calloc(1, sizeof(UnidadDatos));
	if (temp == NULL) return NULL;

	temp->datos = (char *)calloc(tam+contadorDLE+longCab+4, sizeof(char));
	if (temp->datos == NULL) {
		free(temp);
		return NULL;
	}

	tam = tam + contadorDLE + longCab;

	memcpy(temp->datos, inicio, 2); // AÑADO LIMITE INICIO
	
	temp->datos[3] = num_ack;
	temp->datos[4] = 0x10;	temp->datos[5] = 0x02; // DLE STX

	for ( i = longCab, j= 0; i<tam ; i++, j++) {
		temp->datos[i] = trama->datos[j];
		if (trama->datos[j] == 0x10)
			temp->datos[++i] = 0x10;
	}

	temp->datos[i++] = 0x10; temp->datos[i++] = 0x03; // DLE ETX

	// Y el checksum
	checksum = icrc(0,(unsigned char *)temp->datos,tam+1, 255, -1);

	temp->datos[i++] = HIBYTE(checksum);
	temp->datos[i] = LOBYTE(checksum);
	
	temp->longitud = tam + 4;

	return temp;
}
