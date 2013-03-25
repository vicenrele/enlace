/******************************************************************************
 * FICHERO: enlace.c                                                          *
 ******************************************************************************
 *  - Implementación de la función donde se carga la biblioteca "fisico.dll"  * 
 *    y cada una de las primitivas de servicio del nivel de enlace.           *
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "enlace.h"
#include "funciones.h"

#define timeout  1000
#define timeout2 2000


/******************************************************************************
 * Definición de las variables globales de nivel físico.                      *
 ******************************************************************************/
TIPO_F_INICIO_PUERTO f_inicio_puerto;
TIPO_F_ESCRIBIR f_escribir;
TIPO_F_PUEDO_ESCRIBIR f_puedo_escribir;
TIPO_F_LEER f_leer;
TIPO_F_PUEDO_LEER f_puedo_leer;
TIPO_F_ESPERAR_LEER f_esperar_leer;
TIPO_F_ESPERAR_ESCRIBIR f_esperar_escribir;

// Para controlar los números de secuencia
char nSecEsperado, nSecRecibido, nSecEnviar;
// Para contar los números de tramas transmitidas
int nTramasTransmit, nTramasError, nRetTrama, nRetTramaMax;
int tamTramaMax, verTrama;

HINSTANCE biblioteca;

int cargarBiblioteca(char lado)
{
    char clave[] = "idClave";
   
	nSecEsperado = '0';
	nSecRecibido = '0';
	nSecEnviar = '0';
	nTramasTransmit = 0;
	nTramasError = 0;
	nRetTrama = 0;
	int vel, tasaErrores;
	

    biblioteca = LoadLibrary("fisico.dll");
	if (biblioteca == NULL)
	{
		PERROR("LoadLibrary");
		return 1;
	}

	f_inicio_puerto = (TIPO_F_INICIO_PUERTO)GetProcAddress(biblioteca, "f_inicio_puerto");
    if (f_inicio_puerto == NULL)
	{
		PERROR("GetProcAddress: f_inicio_puerto");
		return 1;
	}

    vel = GetPrivateProfileInt("Serie", "Velocidad", 5000, "./enlace.ini");
	tamTramaMax = GetPrivateProfileInt("Enlace", "tamTrama", 1200, "./enlace.ini");
	tasaErrores = GetPrivateProfileInt("DLL", "Errores", 0, "./enlace.ini");
	verTrama = GetPrivateProfileInt("Preferencias", "verTramasYprimitivas", 1, "./enlace.ini");
	
	// Se inicia el puerto
	if (f_inicio_puerto (lado, NULL, clave, vel, NULL, tasaErrores) == FALSE)
	{
		fprintf(stderr, "Error: función f_inicio_puerto\n");
		fflush(stderr);
		return 1;
	}
	
    f_escribir = (TIPO_F_ESCRIBIR)GetProcAddress(biblioteca, "f_escribir");
    if (f_escribir == NULL)
	{
		PERROR("GetProcAddress: f_escribir");
		return 1;
	}
	
    f_puedo_escribir = (TIPO_F_PUEDO_ESCRIBIR)GetProcAddress(biblioteca, "f_puedo_escribir");
    if (f_puedo_escribir == NULL)
	{
		PERROR("GetProcAddress: f_puedo_escribir");
		return 1;
	}
	
	f_leer = (TIPO_F_LEER)GetProcAddress(biblioteca, "f_leer");
    if (f_leer == NULL)
	{
		PERROR("GetProcAddress: f_leer");
		return 1;
	}

    f_puedo_leer = (TIPO_F_PUEDO_LEER)GetProcAddress(biblioteca, "f_puedo_leer");
    if (f_puedo_leer == NULL)
	{
		PERROR("GetProcAddress: f_puedo_leer");
		return 1;
	}

    f_esperar_leer = (TIPO_F_ESPERAR_LEER)GetProcAddress(biblioteca, "f_esperar_leer");
    if (f_esperar_leer == NULL)
	{
		PERROR("GetProcAddress: f_esperar_leer");
		return 1;
	}

    f_esperar_escribir = (TIPO_F_ESPERAR_ESCRIBIR)GetProcAddress(biblioteca, "f_esperar_escribir");
    if (f_esperar_escribir == NULL)
	{
		PERROR("GetProcAddress: f_esperar_escribir");
		return 1;
	}	
	
	return 0;
}

/******************************************************************************
 * Primitiva que genera una trama de conexión y la envía.              - P1 - *
 ******************************************************************************/
int L_CONNECT_request(int origen, int destino, int prioridad)
{
	char trama[1500];
	int tam, i, valorRet;
	
	// Muetra por pantalla el la primitiva invocada
    if (verTrama)
    {
	    fprintf(stdout, "\nPrimitiva Invocada: L_CONNECT_request\n"); 
	    fflush(stdout);
    }
	solicitarConexion(trama, &tam);
	
	for (i=0; i < tam; i++)
	{
	    // Espera hasta que se pueda enviar un carácter
        if (f_esperar_escribir(INFINITE) != 1) 
	    {
            fprintf(stderr, "ERROR: función f_esperar_escribir (L_CONNECT_request)\n");
		    fflush(stderr);
		    return 1;
		}
		// Envía el carácter
	    if (f_escribir(trama[i]) != 0) 
	    {
            fprintf(stderr, "ERROR: función f_escribir (L_CONNECT_request)\n");
		    fflush(stderr);
		    return 1;
		}
	}
    
	if (verTrama)
    {
	    fprintf(stdout, "\n    Trama Enviada ..........: SOLICITUD DE CONEXION\n");
        fflush(stdout);
        mostrarTrama(trama, tam);
    }
	
	return 0;
}

/******************************************************************************
 * Primitiva que reconoce una trama de solicitud de conexión.          - P2 - *
 ******************************************************************************/
int L_CONNECT_indication(int origen, int destino, int prioridad)
{
	char trama[tamTramaMax], datos[tamTramaMax];
	int tam, tamDatos, valorRet, ret;
	int i = 0;
	
	if (verTrama) 
	{
	    fprintf(stdout, "\nPrimitiva Invocada: L_CONNECT_indication\n"); 
	    fflush(stdout);
    }

    // Espera hasta que se pueda recibir el primer carácter
    ret = f_esperar_leer(INFINITE);
	switch(ret)
	{
	    case 0: 
            fprintf(stderr, "ERROR: f_esperar_leer: No hay caracter y la conexión sigue activa (L_CONNECT_indication)\n");
			fflush(stderr); 
            return 1;
        case 1: 
       	    // Cuando se pueda, lee el carácter y lo almacena en trama[i]
            switch(f_leer(&trama[i]))
	        {
	            case 0:
                    i++;
			        break;
	            case -1:
                    fprintf(stderr, "Error: función f_leer sobreescritura (L_CONNECT_indication)\n");
			        fflush(stderr);
                    return 1;
	            case -2:
                    fprintf(stderr, "Error: función f_leer no hay nada que leer (L_CONNECT_indication)\n");
			        fflush(stderr);
                    return 1;
	        }
	        break;
	    case 2: 
            fprintf(stderr, "ERROR: f_esperar_leer: Se ha cortado la conexión (L_CONNECT_indication)\n");
			fflush(stderr); 
            return 1;
	    case 3: 
            fprintf(stderr, "ERROR: f_esperar_leer: Hay caracter para leer y se ha cortado la conexión (L_CONNECT_indication)\n");
			fflush(stderr); 
            return 1;    
	}

	do
	{
        // Espera hasta que se pueda recibir un carácter o venza el timeout
	    ret = f_esperar_leer(timeout);
	    switch(ret)
	    {
		    case 0: 
                fprintf(stderr, "ERROR: f_esperar_leer: No hay caracter y la conexión sigue activa (L_CONNECT_indication)\n");
				fflush(stderr); 
                return 1;
	        case 1: 
                // Cuando se pueda, lee el carácter y lo almacena en trama[i]
                switch(f_leer(&trama[i]))
		        {
		            case 0:
                        i++;
				        break;
		            case -1:
                        fprintf(stderr, "ERROR: función f_leer sobreescritura (L_CONNECT_indication)\n");
			            fflush(stderr); 
                        return 1;
		            case -2:
                        fprintf(stderr, "ERROR: función f_leer no hay nada que leer (L_CONNECT_indication)\n");
				        fflush(stderr); 
                        return 1;
		        }
		        break;
    	    case 2: 
                 fprintf(stderr, "ERROR: f_esperar_leer: Se ha cortado la conexión (L_CONNECT_indication)\n");
			     fflush(stderr); 
                 return 1;
	        case 3: 
                 fprintf(stderr, "ERROR: f_esperar_leer: Hay caracter para leer y se ha cortado la conexión (L_CONNECT_indication)\n");
			     fflush(stderr); 
                 return 1;   
	        case 4: 
                 // Cuando vence el timeout, se analiza la trama para ver si se ha recibido una trama de conexión
                 tam = i;
				 valorRet = analizarTrama(trama, &tam , datos, &tamDatos);						
				 				 
				 if (valorRet == 2)
				 {
			         // Verificamos el tipo de la trama y la mostramos
                     if (verTrama)
                     {
					     tipoTrama(valorRet);
				         mostrarTrama(trama, tam);
				     }
				     // Incrementamos el número de tramas transmitidas
				     nTramasTransmit++;
	                 return 0;
				 }			
                 else
                 {
				     fprintf(stdout, "\nERROR. No se ha recibido una solicitud de conexion\n"); 
	                 fflush(stdout);
	                 // Incrementamos el número de tramas transmitidas y erróneas
				     nTramasTransmit++;
					 nTramasError++;
					 return 1;
				 }
				     
        }
    } while(ret != 4);    // Hasta que no venza el timeout
}

/******************************************************************************
 * Genera una trama de confirmación de conexión y la envía.            - P3 - *
 ******************************************************************************/	
int L_CONNECT_response(int origen, int destino, int prioridad)
{
	char trama[tamTramaMax];
	int tam, i;
	
	if (verTrama)
	{
	    fprintf(stdout, "\n\nPrimitiva Invocada: L_CONNECT_response\n"); 
	    fflush(stdout);
    }
	// Genera una trama de aceptación de conexión
    conexionAceptada(trama, &tam);
		
	for (i=0; i < tam; i++)
	{
        // Espera hasta que se pueda enviar un carácter
	    if (f_esperar_escribir(INFINITE) != 1) 
	    {
            fprintf(stderr, "ERROR: función f_esperar_escribir (L_CONNECT_response)\n");
		    fflush(stderr);
		    return 1;
		}
		// Envía el carácter
	    if (f_escribir(trama[i]) != 0) 
	    {
            fprintf(stderr, "ERROR: función f_escribir (L_CONNECT_response)\n");
		    fflush(stderr);
		    return 1;
		}
	}
	if (verTrama)
	{
	    fprintf(stdout, "\n    Trama Enviada ..........: CONEXION ACEPTADA\n");
        fflush(stdout);
        mostrarTrama(trama, tam);
    }
	return 0;

}

/******************************************************************************
 * Primitiva que acepta la conexión.                                 - P4 - *
 ******************************************************************************/
int L_CONNECT_confirm(int origen, int destino, int prioridad)
{
	char trama[tamTramaMax], datos[tamTramaMax];
	int tam, tamDatos, valorRet, ret;
	int i = 0;
	
    if (verTrama)
    {
	    fprintf(stdout, "\n\nPrimitiva Invocada: L_CONNECT_confirm\n"); 
	    fflush(stdout);
    }
    // Espera hasta que se pueda recibir el primer carácter
    ret = f_esperar_leer(INFINITE);
	switch(ret)
	{
	    case 0: 
            fprintf(stderr, "ERROR: f_esperar_leer: No hay caracter y la conexión sigue activa (L_CONNECT_confirm)\n");
			fflush(stderr); 
            return 1;
	    case 1: 
            // Cuando se pueda, lee el carácter y lo almacena en trama[i]
            switch(f_leer(&trama[i]))
	        {
	            case 0:
                    i++;
			        break;
	            case -1:
                    fprintf(stderr, "ERROR: función f_leer sobreescritura (L_CONNECT_confirm)\n");
		            fflush(stderr);
                    return 1;
	            case -2:
                    fprintf(stderr, "ERROR: función f_leer no hay nada que leer (L_CONNECT_confirm)\n");
			        fflush(stderr);
                    return 1;
	        }
	        break;
		case 2: 
            fprintf(stderr, "ERROR: f_esperar_leer: Se ha cortado la conexión (L_CONNECT_confirm)\n");
			fflush(stderr); 
            return 1;
		case 3: 
            fprintf(stderr, "ERROR: f_esperar_leer: Hay caracter para leer y se ha cortado la conexión (L_CONNECT_confirm)\n");
			fflush(stderr); 
            return 1;
	}
	
    do
	{
		// Espera hasta que se pueda recibir un carácter
        ret=f_esperar_leer(timeout);
		switch(ret)
		{
	 	    case 0: 
                fprintf(stderr, "ERROR: f_esperar_leer: No hay caracter y la conexión sigue activa (L_CONNECT_confirm)\n");
			    fflush(stderr); 
                return 1;
		    case 1:
                // Cuando se pueda, lee el carácter y lo almacena en trama[i] 
                switch(f_leer(&trama[i]))
		        {
			        case 0:
                        i++;
				        break;
			        case -1:
                        fprintf(stderr, "ERROR: función f_leer sobreescritura (L_CONNECT_confirm)\n");
				        fflush(stderr);
                        return 1;
			        case -2:
                        fprintf(stderr, "ERROR: función f_leer no hay nada que leer (L_CONNECT_confirm)\n");
			            fflush(stderr); 
                        return 1;
		        }
		        break;
		    case 2: 
                fprintf(stderr, "ERROR: f_esperar_leer: Se ha cortado la conexión (L_CONNECT_confirm)\n");
			    fflush(stderr); 
                return 1;
		    case 3: 
                fprintf(stderr, "ERROR: f_esperar_leer: Hay caracter para leer y se ha cortado la conexión (L_CONNECT_confirm)\n");
				fflush(stderr);
                return 1;
		    case 4: 
                tam = i; 
				// Cuando vence el timeout, se analiza la trama para ver si se ha aceptado la conexión
                valorRet = analizarTrama(trama, &tam , datos, &tamDatos);
				// Dice el tipo de trama que ha llegado y muestra la trama
                
				if (valorRet == 5)
				{
				    if (verTrama)
				    {
					    tipoTrama(valorRet);   			 
				   	    mostrarTrama(trama, tam);
				    }
                    // Se incrementa el número de tramas transmitidas
			        nTramasTransmit++;
                    return 0;
				}
				else
				{
					fprintf(stdout, "\nERROR. No se ha recibido una aceptación de conexion\n"); 
	                fflush(stdout);
	                // Se incrementa el número de tramas transmitidas y erróneas
                    nTramasTransmit++;
					nTramasError++;
				    return 1;
				}	
		}
	} while(ret!=4);    // Hasta que no venza el timeout						
}

/******************************************************************************
 * Primitiva que genera una trama de datos y la envía.                 - P5 - *
 ******************************************************************************/
int  L_DATA_request(int origen, int destino, char *datos, int tamDatos)
{
	char trama[tamTramaMax], tramaConfirm[tamTramaMax];
	int tam, tamConfirm, valorRet, ret;
	int i, exito = 1;

	// Muestra el nombre de la primitiva invocada
    if (verTrama)
    {
	    fprintf(stdout, "\nPrimitiva Invocada: L_DATA_request\n"); 
	    fflush(stdout);
    }
	while (exito)
    {
        // Se genera una trama de datos para transmisión
		tramaDatos(trama, &tam, datos, &tamDatos, nSecEnviar); 
		
		// Se envia la trama
		for (i = 0; i < tam; i++)
		{
			// Espera hasta que se pueda enviar un carácter
            if (f_esperar_escribir(INFINITE) != 1)
			{
				fprintf(stderr, "ERROR: función f_esperar_escribir (L_DATA_request)\n");
				fflush(stderr);
				return 1;
			}
			// Envía el carácter
			if (f_escribir(trama[i]) != 0)
			{
				fprintf(stderr, "ERROR: función f_escribir (L_DATA_request)\n");
				fflush(stderr);
				return 1;
			}
		}
		
		// Muestra por pantalla la trama enviada
		if (verTrama)
		{
		    fprintf(stdout, "\n    Trama Enviada ..........: TRAMA DE DATOS\n");
            fflush(stdout);
            mostrarTrama(trama, tam);
		    fprintf(stdout, "\n    Caracteres CRC .........: %c %c (Enviados)\n", trama[tam-2], trama[tam-1]); 
            fflush(stdout);
        }
        // Espera hasta que se pueda recibir el primer carácter
		i = 0;
        ret = f_esperar_leer(timeout2);
		switch(ret)
		{
	        case 0:
                fprintf(stderr, "ERROR: f_esperar_leer: No hay caracter y la conexión sigue activa (L_DATA_request)\n");
			    fflush(stderr);
                return 1;
		    case 1:
                // Cuando se pueda, lee el carácter y lo almacena en tramaConfirm[i]
     	        switch(f_leer(&tramaConfirm[i]))
		        {
			        case 0:
                        i++;
				        break;
			        case -1:
                        fprintf(stderr, "ERROR: función f_leer sobreescritura (L_DATA_request)\n");
				        fflush(stderr);
                        return 1;
			        case -2:
                        fprintf(stderr, "ERROR: función f_leer no hay nada que leer (L_DATA_request)\n");
				        fflush(stderr); 
                        return 1;
		        }
		        break;
		    case 2:
                fprintf(stderr, "ERROR: f_esperar_leer: Se ha cortado la conexión (L_DATA_request)\n");
			    fflush(stderr);
                return 1;
		    case 3:
                fprintf(stderr, "ERROR: f_esperar_leer: Hay caracter para leer y se ha cortado la conexión (L_DATA_request)\n");
			    fflush(stderr);
                return 1;
		}
	
		do
		{
            // Espera hasta que se pueda recibir un carácter o venza el timeout
		    ret = f_esperar_leer(timeout);
		    switch(ret)
			{
			    case 0:
                    fprintf(stderr, "ERROR: f_esperar_leer: No hay caracter y la conexión sigue activa (L_DATA_request)\n");
					fflush(stderr);
                    return 1;
			    case 1:
                    // Cuando se pueda, lee el carácter y lo almacena en tramaConfirm[i]
                    switch(f_leer(&tramaConfirm[i]))
				    {
				        case 0:
                            i++;
						    break;
				        case -1:
                            fprintf(stderr, "ERROR: función f_leer sobreescritura (L_DATA_request)\n");
						    fflush(stderr);
                            return 1;
				        case -2:
                            fprintf(stderr, "ERROR: función f_leer no hay nada que leer (L_DATA_request)\n");
						    fflush(stderr);
                            return 1;
			        }
					break;
			    case 2:
                    fprintf(stderr, "ERROR: f_esperar_leer: Se ha cortado la conexión (L_DATA_request)\n");
					fflush(stderr);
                    return 1;
			    case 3:
                    fprintf(stderr, "ERROR: f_esperar_leer: Hay caracter para leer y se ha cortado la conexión (L_DATA_request)\n");
					fflush(stderr);
                    return 1;
			    case 4:
                    tamConfirm = i;
                    valorRet = analizarTrama(tramaConfirm, &tamConfirm , datos, &tamDatos);
                    if (valorRet == 6)
                    {
					    if (verTrama)
					    {
						    tipoTrama(valorRet);
						    mostrarTrama(tramaConfirm, tamConfirm);
						    fprintf(stdout,"\n");
					        fflush(stdout);
					    }
					    // Se incrementa el número de tramas transmitidas
						nTramasTransmit++;
					    if (nRetTramaMax < nRetTrama)
                        {
			                nRetTramaMax = nRetTrama;
			                nRetTrama = 0;
			            }
					    				        
                        // Si el número de secuencia a enviar es el recibido, lo cambiamos
                        nSecRecibido = tramaConfirm[6];
					    if (nSecEnviar != nSecRecibido && nSecEnviar == '0')
                        {
						    nSecEnviar = '1';
						    exito = 0;
					    }
						if (nSecEnviar != nSecRecibido && nSecEnviar == '1')
                        {
							nSecEnviar = '0';
							exito = 0;
						}
						else
					        exito = 0;
					}
					else
					{
					    fprintf(stderr, "ERROR: Trama de confirmación incorrecta. Reenviamos la trama de datos. (L_DATA_request)\n");
					    fflush(stderr);
					    // Incrementamos el número de tramas transmitidas y erróneas
				        nTramasTransmit++;
						nTramasError++;
						nRetTrama++;
					    exito = 1;
					}
					break;
            }
        } while (ret != 4);    // Hasta que no venza el timeout
    }    // Hasta que éxito se ponga a 0
	return 0;
}

/******************************************************************************
 * Primitiva que recorre los datos de una trama de datos recibida.     - P6 - *
 ******************************************************************************/
int L_DATA_indication(int origen, int destino, char *datos, int *tamDatos)
{
	char trama[tamTramaMax];
    char tramaConfirm[tamTramaMax];
	int tam, tamConfirm, valorRet, ret, crcRet;
	int i = 0; 
	int exito = 1;
     
    // Muestra el nombre de la primitiva invocada
	if (verTrama)
	{
	    fprintf(stdout, "\n\nPrimitiva Invocada: L_DATA_indication\n"); 
	    fflush(stdout);
	}
    // Espera hasta que se pueda recibir el primer carácter
    while (exito)
    {
        ret = f_esperar_leer(INFINITE);
	    switch(ret)
	    {
	        case 0:
                fprintf(stderr, "ERROR: f_esperar_leer: No hay caracter y la conexión sigue activa (L_DATA_indication)\n");
			    fflush(stderr);
                return 1;
	        case 1:
    	        // Cuando se pueda, lee el carácter y lo almacena en trama[i]
                switch(f_leer(&trama[i]))
	            {
	                case 0:
                        i++;
			            break;
	                case -1:
                        fprintf(stderr, "ERROR: función f_leer sobreescritura (L_DATA_indication)\n");
			            fflush(stderr);
                        return 1;
	                case -2:
                        fprintf(stderr, "ERROR: función f_leer no hay nada que leer (L_DATA_indication)\n");
			            fflush(stderr);
                        return 1;
	            }
	            break;
	        case 2:
                fprintf(stderr, "ERROR: f_esperar_leer: Se ha cortado la conexión (L_DATA_indication)\n");
			    fflush(stderr);
                return 1;
	        case 3:
                fprintf(stderr, "ERROR: f_esperar_leer: Hay caracter para leer y se ha cortado la conexión (L_DATA_indication)\n");
			    fflush(stderr);
                return 1;
	    }

	    do
	    {
	        ret = f_esperar_leer(timeout);
	        switch(ret)
		    {
		        case 0:
                    fprintf(stderr, "ERROR: f_esperar_leer: No hay caracter y la conexión sigue activa (L_DATA_indication)\n");
				    fflush(stderr);
                    return 1;
		        case 1:
                    switch(f_leer(&trama[i]))
			        {
				        case 0:
                            i++;
					        break;
				        case -1:
                            fprintf(stderr, "ERROR: función f_leer sobreescritura (L_DATA_indication)\n");
					        fflush(stderr);
                            return 1;
				        case -2:
                            fprintf(stderr, "ERROR: función f_leer no hay nada que leer (L_DATA_indication)\n");
					        fflush(stderr);
                            return 1;
			        }
			        break;
		        case 2:
                    fprintf(stderr, "ERROR: f_esperar_leer: Se ha cortado la conexión (L_DATA_indication)\n");
			        fflush(stderr);
                    return 1;
		        case 3:
                    fprintf(stderr, "ERROR: f_esperar_leer: Hay caracter para leer y se ha cortado la conexión (L_DATA_indication)\n");
			        fflush(stderr);
                    return 1;
		        case 4:
                    tam = i;
                
                    // Cuando vence el timeout, se analiza la trama para ver si ha llegado una trama de datos o desconexión
				    valorRet = analizarTrama(trama, &tam , datos, tamDatos);
                
                    if (valorRet == 4)
                    {
			            if (verTrama)
			            {
					        tipoTrama(valorRet);
					        mostrarTrama(trama, tam);
					        fprintf(stdout, "\n");
			                fflush(stdout);
				        }
					    // Se incrementa el número de tramas transmitidas
                        nTramasTransmit++;
                        return 2;
				    }
				    // Ha llegado una trama de datos
				    if (valorRet == 7)
                    {
  		                if (verTrama)
  		                {
					        tipoTrama(valorRet);
					        mostrarTrama(trama, tam);
				        }
					    // Comprobamos los códigos de control
					    if (validarCrc (trama, &tam))
			            {
	   			    	    fprintf(stdout, "\n\nERROR. Comprobacion CRC incorrecta: %c %c\n", trama[tam-2], trama[tam-1]);
			                fflush(stdout);
			                nTramasError;
                        }
				        else 
	                    {
					 	    if (verTrama)
					 	    {
						        fprintf(stdout, "\n    Caracteres CRC .........: %c %c (Los recibidos son iguales a los enviados)", trama[tam-2], trama[tam-1]); 
                                fflush(stdout);
				            }
                            exito = 0;    // Se sale del bucle while 
				        }
			    	 
				        // Se incrementa el número de tramas transmitidas
                        nTramasTransmit++;
                        nSecRecibido = trama[6];
                        confirmacionTrama(tramaConfirm, &tamConfirm);
                        if(nSecEsperado == nSecRecibido)    // Ha ido bien
                        {
                            if (nSecRecibido == '0') 
                            {
                                tramaConfirm[6] = '1';
                                nSecEsperado = '1';
                                exito = 0;
                            }
                            if (nSecRecibido == '1')
                            {
                                tramaConfirm[6] = '0';
                                nSecEsperado = '0';
                                exito = 0;
                            }
                        }
                        else
                        {
                            if (nSecRecibido == '0')
					            tramaConfirm[6] = '0';
	                        else
	                            tramaConfirm[6] = '1';
                        }
                    										                   
                        tamConfirm = 7;
                        for (i = 0; i < tamConfirm; i++)
				        {
					    // Espera hasta que pueda enviar un carácter
                            if (f_esperar_escribir(INFINITE) != 1)
						    {
						        fprintf(stderr, "ERROR: función f_esperar_escribir (L_DATA_indication)\n");
							    fflush(stderr);
							    return 1;
						    }
						    // Envía el carácter
						    if (f_escribir(tramaConfirm[i]) != 0)
						    {
							    fprintf(stderr, "ERROR: función f_escribir (L_DATA_indication)\n");
							    fflush(stderr);
							    return 1;
						    }
                        } 
                    } // Fin if (trama de datos)
		    	    else
		    	    {
				        fprintf(stdout, "ERROR: Trama Incorrecta (L_DATA_indication)\n");
					    fflush(stdout);
					    nSecRecibido = trama[6];
                        confirmacionTrama(tramaConfirm, &tamConfirm);
                        if(nSecEsperado == nSecRecibido)    // Ha ido bien
                        {
                            if (nSecRecibido == '0') 
                            {
                                tramaConfirm[6] = '0';
                                nSecEsperado = '1';
                                exito = 0;
                            }
                            if (nSecRecibido == '1')
                            {
                                tramaConfirm[6] = '1';
                                nSecEsperado = '0';
                                exito = 0;
                            }
                        }
                        else
                        {
                            if (nSecRecibido == '0')
					            tramaConfirm[6] = '0';
	                        else
	                            tramaConfirm[6] = '1';
                        }
                    										                   
                        tamConfirm = 7;
                        for (i = 0; i < tamConfirm; i++)
				        {
					    // Espera hasta que pueda enviar un carácter
                            if (f_esperar_escribir(INFINITE) != 1)
						    {
						        fprintf(stderr, "ERROR: función f_esperar_escribir (L_DATA_indication)\n");
							    fflush(stderr);
							    return 1;
						    }
						    // Envía el carácter
						    if (f_escribir(tramaConfirm[i]) != 0)
						    {
							    fprintf(stderr, "ERROR: función f_escribir (L_DATA_indication)\n");
							    fflush(stderr);
							    return 1;
						    }
                        }
                        // Incrementamos el número de tramas transmitidas y erróneas
				        nTramasTransmit++;
                        nTramasError++;
                    }
			        break;
		    } // Fin switch
        } while (ret != 4);    // Hasta que venza el timeout y exito se ponga a 0
    } // Fin while (éxito)
    return 0;
}

/******************************************************************************
 * Genera una trama de desconexión y la envía.                         - P7 - *
 ******************************************************************************/	
int L_DISCONNECT_request(int origen, int destino , int prioridad)
{
	char trama[tamTramaMax];
	int tam, i;
	
	// Muestra el nombre de la primitiva invocada
    if (verTrama)
    {
	    fprintf(stdout, "\nPrimitiva Invocada: L_DISCONNECT_request\n\n"); 
	    fflush(stdout);
	}
	
	// Mostramos el número de tramas transmitidas
	fprintf(stdout, "\n\n");
    fflush(stdout);
    fprintf(stdout, "ESTADISTICAS:\n");
    fflush(stdout);
	fprintf(stdout, "    Tramas Transmitidas ....: %d (Recibidas)\n", nTramasTransmit);
    fflush(stdout);
    fprintf(stdout, "    Tramas Erroneas ........: %d\n", nTramasError);
    fflush(stdout);
    fprintf(stdout, "    Max. Retransmisiones ...: %d\n", nRetTramaMax);
    fflush(stdout);
    fprintf(stdout, "\n");
    fflush(stdout);

	// Genera una trama de desconexión
	desconexion(trama, &tam);
	
	for (i = 0; i < tam; i++)
	{
	    // Espera hasta que pueda enviar un carácter
        if (f_esperar_escribir(INFINITE) != 1) 
	    {
            fprintf(stderr, "Error: función f_esperar_escribir (L_DISCONNECT_request)\n");
		    fflush(stderr);
		    return 1;
		}
		// Envía un carácter
	    if (f_escribir(trama[i]) != 0) 
	    {
            fprintf(stderr, "Error: función f_escribir (L_DISCONNECT_request)\n");
		    fflush(stderr);
		    return 1;
		}
	}
	return 0;
}

/******************************************************************************
 * Primitiva L_DISCONNECT_indication: No realiza ninguna acción.       - P8 - *
 ******************************************************************************/
void L_DISCONNECT_indication(int origen, int destino, int prioridad)
{ 
    // Muestra el nombre de la primitiva invocada
    if (verTrama)
    {
	    fprintf(stdout, "\nPrimitiva Invocada: L_DISCONNECT_indication\n\n"); 
	    fflush(stdout);
    }
	fprintf(stdout, "\n\n");
    fflush(stdout);
    fprintf(stdout, "ESTADISTICAS:\n");
    fflush(stdout);
	fprintf(stdout, "    Tramas Transmitidas ....: %d (Recibidas)\n", nTramasTransmit);
    fflush(stdout);
    fprintf(stdout, "    Tramas Erroneas ........: %d\n", nTramasError);
    fflush(stdout);
    fprintf(stdout, "    Max. Retransmisiones ...: %d\n", nRetTramaMax);
    fflush(stdout);
}


