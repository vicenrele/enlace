/******************************************************************************
 * FICHERO: enlace.h                                                          *
 ******************************************************************************
 *  - Prototipos de las funciones del nivel de enlace.                        *
 ******************************************************************************/

// Prototipo de las funciones del servicio de enlace de conexión
int L_CONNECT_request(int,int,int);
int L_CONNECT_indication(int,int,int);
int L_CONNECT_response(int,int,int);
int L_CONNECT_confirm(int,int,int);
int L_DISCONNECT_request(int,int,int);
void L_DISCONNECT_indication(int,int,int);    
int L_DATA_request(int,int,char *,int);
int L_DATA_indication(int,int,char *,int *);
int L_RESET_indication(int,int,int);
