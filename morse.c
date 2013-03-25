/******************************************************************************
 * FICHERO: morse.c                                                           *
 ******************************************************************************
 *  - Nivel de aplicación.                                                    *
 ******************************************************************************/

#include <stdio.h>
#include <string.h>   

#include "funciones.h"
#include "windows.h"

int ladoIzquierdo(void);
int ladoDerecho(void);
void codMorse(char *frase, char *fraseConvert);

/******************************************************************************
 * Cargamos la biblioteca, comprobamos los argumentos y llamamos a las        *
 * funciones para que inicien la comunicación.                                *
 ******************************************************************************/
int main (int argc, char *argv[])
{
	if (argc != 2)
    {
		fprintf(stderr,"USO: %s I|D\n", argv[0]);
        fflush(stderr);
		return 1;
	}
	if(cargarBiblioteca(argv[1][0])==1)
    {
        fprintf(stderr, "ERROR: cargarBiblioteca\n");
		fflush(stderr);
        return 1;
    }
	if(toupper(argv[1][0]) == 'I')       
        if(ladoIzquierdo()==1){
            fprintf(stderr, "ERROR: ladoIzquierdo\n"); 
            fflush(stderr); 
            return 1;
        } 
    if(toupper(argv[1][0]) == 'D')
        if(ladoDerecho()==1){
            fprintf(stderr, "ERROR: ladoDerecho\n"); 
            fflush(stderr); 
            return 1;
        }
return 0;
}

/******************************************************************************
 * Función ladoIzquierdo(Receptor)                                            *
 ******************************************************************************/
int ladoIzquierdo(void)
{
	char frase[1500], fraseConvert[1500];
	int tamFrase, tamFraseConvert; 

    // Recibe del otro extremo una solicitud de conexión
    if(L_CONNECT_indication(0,0,0)==1)
    {
		fprintf(stderr, "ERROR: L_CONNECT_indication(ladoIzquierdo)\n"); 
		fflush(stderr); 
        return 1;
	}
	// Envía una confirmación de conexión
	if(L_CONNECT_response(0,0,0)==1)
    {
		fprintf(stderr, "ERROR: L_CONNECT_response(ladoIzquierdo)\n"); 
		fflush(stderr); 
        return 1;
	}
    for(;;)
    {
        // Recibe del otro extremo una frase, la convierte a Morse y la reenvía
        switch(L_DATA_indication(0,0,frase,&tamFrase))
        {
			case 0:
                frase[tamFrase] = '\0';
				codMorse(frase, fraseConvert);
				tamFraseConvert = strlen(fraseConvert);
                fraseConvert[tamFraseConvert] = '\0';
                fprintf(stdout, "\n    Frase Recibida .........: %s\n", frase); 
                fflush(stdout);
			    Sleep(3000);
                
                fprintf(stdout, "\n       ... Enviando %s ...\n", fraseConvert);
		        fflush(stdout);
                
                if (L_DATA_request(0,0,fraseConvert,tamFraseConvert))
			    {
                    fprintf(stderr, "ERROR: L_DATA_request(ladoIzquierdo). No se ha podido enviar %s\n", fraseConvert);
					fflush(stderr);
                    return 1;
			    }
                break;
			case 1: 
                fprintf(stderr, "ERROR: L_DATA_indication(ladoIzquierdo)\n"); 
				fflush(stderr); 
                return 1;   
			case 2:
                // Si recibe un EOT, desconecta 
                L_DISCONNECT_indication(0,0,0);
				return 0;
        }
   	}
    return 0;
}

/******************************************************************************
 * Función ladoDerecho(Emisor)                                                *
 ******************************************************************************/
int ladoDerecho(void)
{
	char frase[1500], fraseConvert[1500];
	int tamFrase, tamFraseConvert; 

    // Envía una solicitud de conexión al otro extremo (ladoIzquierdo)
    if(L_CONNECT_request(0,0,0) == 1)
    {
		fprintf(stderr, "ERROR: L_CONNECT_request(ladoDerecho)\n"); 
		fflush(stderr); 
        return 1;
	}
	// Acepta la confirmación de conexión
	if(L_CONNECT_confirm(0,0,0)==1){
		fprintf(stderr, "ERROR: L_CONNECT_confirm(ladoDerecho)\n"); 
		fflush(stderr); 
        return 1;
	}
	for(;;)
    {
        fprintf(stdout, "\n\nIntroduzca una frase para convertirla a morse o FIN para terminar: ");
        fflush(stdout);
        
        // Pide por teclado una cadena
        gets(frase);
        tamFrase = strlen(frase);

        // Si la cadena introducida es distinta de 'FIN' la envía al otro extremo
        if (strcmp(frase, "FIN") != 0)
        {
            // Envía frase
            fprintf(stdout, "\n       ... Enviando %s ...\n", frase);
		    fflush(stdout);
            
			if (L_DATA_request(0,0,frase,tamFrase) == 1)
            {
                fprintf(stderr, "ERROR: L_DATA_request(ladoDerecho). No se ha podido enviar %s\n", frase); 
		        fflush(stderr); 
                return 1;
	        }
	        
           // Recibe la trama en Morse
           switch (L_DATA_indication(0,0,fraseConvert,&tamFraseConvert))
			{
                case 0:
                    
                    fraseConvert[tamFraseConvert] = '\0';
                    fprintf(stdout, "\n    Frase Recibida .........: %s", fraseConvert); 
                    fflush(stdout);
					break;

                case 1:
                    fprintf(stderr, "ERROR: L_DATA_indication(ladoDerecho)\n");
					fflush(stderr);
                    return 1;
				case 2:
                    // Si recibe un EOT, desconecta
                    L_DISCONNECT_indication(0,0,0);
				    return 0;
				} 
	    }
        // Si la cadena introducida es igual a 'FIN' solicita al otro extremo la desconexión   
	    else
        {
	        fprintf(stdout, "\n       ... Enviando EOT ...\n");
	        fflush(stdout);
			
			if (L_DISCONNECT_request(0,0,0) == 1)
            {
                fprintf(stderr, "ERROR: L_DISCONNECT_request(ladoDerecho)\n"); 
		        fflush(stderr); 
                return 1;
            }
            Sleep(2000);
            return 0; 
		}
    }
}

/******************************************************************************
 * Función que recibe una cadena de caracteres y la codifica a código Morse.  *
 ******************************************************************************/
void codMorse(char *frase, char *fraseConvert)
{
    fraseConvert[0] = '\0';  
    int i = 0;

    for (i=0; i<(strlen(frase)); i++)
    {
	    switch(toupper(frase[i]))
        {
            // Letras
            case 'A': strcat(fraseConvert, ".-");     break;
            case 'B': strcat(fraseConvert, "-...");   break;
            case 'C': strcat(fraseConvert, "-.-.");   break;
            case 'D': strcat(fraseConvert, "-..");    break;
            case 'E': strcat(fraseConvert, ".");      break;
            case 'F': strcat(fraseConvert, "..-.");   break;
            case 'G': strcat(fraseConvert, "--.");    break;
            case 'H': strcat(fraseConvert, "....");   break;
            case 'I': strcat(fraseConvert, "..");     break;
            case 'J': strcat(fraseConvert, "-.-.");   break;
            case 'K': strcat(fraseConvert, "-.-");    break;
            case 'L': strcat(fraseConvert, ".-..");   break;
            case 'M': strcat(fraseConvert, "--");     break;
            case 'N': strcat(fraseConvert, "-.");     break;
            case 'O': strcat(fraseConvert, "---");    break;
            case 'P': strcat(fraseConvert, ".--.");   break;
            case 'Q': strcat(fraseConvert, "--.-");   break;
            case 'R': strcat(fraseConvert, ".-.");    break;
            case 'S': strcat(fraseConvert, "...");    break;
            case 'T': strcat(fraseConvert, "-");      break;
            case 'U': strcat(fraseConvert, "..-");    break;
            case 'V': strcat(fraseConvert, "...-");   break;
            case 'W': strcat(fraseConvert, ".--");    break;
            case 'X': strcat(fraseConvert, "-..-");   break;
            case 'Y': strcat(fraseConvert, "-.--");   break;
            case 'Z': strcat(fraseConvert, "--..");   break;

            // Dígitos
            case '0': strcat(fraseConvert, "-----");  break;
            case '1': strcat(fraseConvert, ".----");  break;
            case '2': strcat(fraseConvert, "..---");  break;
            case '3': strcat(fraseConvert, "...--");  break;
            case '4': strcat(fraseConvert, "....-");  break;
            case '5': strcat(fraseConvert, ".....");  break;
            case '6': strcat(fraseConvert, "-....");  break;
            case '7': strcat(fraseConvert, "--...");  break;
            case '8': strcat(fraseConvert, "---..");  break;
            case '9': strcat(fraseConvert, "----.");  break;

            // Caracteres especiales
            case '.': strcat(fraseConvert, ".-.-.-"); break;
            case ',': strcat(fraseConvert, "--..--"); break;
            case '?': strcat(fraseConvert, "..--.."); break;
            // Si hay un espacio ponemos otro #
			case ' ': strcat(fraseConvert, "#");      break;
  
            default: 
                printf("Caracter invalido: '%c'\n", frase[i]); 
                break;
        }
  
        // Introduce el carácter separador(#) entre cada carácter
        if (i < (strlen(frase) - 1))
            strcat(fraseConvert, "#");
    }
}

