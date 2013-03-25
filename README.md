enlace
===========
Se trata de una aplicación windows que implementa una entidad de enlace. 
* Permite comprender el concepto de abstracción de niveles en el modelo de referencia OSI o en 
cualquier otro modelo arquitectónico de redes.
* Se ha codificado un protocolo simple de nivel de enlace de acuerdo con las especificaciones 
de las normas OSI.
* Se ha implementado una biblioteca de funciones de acceso al nivel de enlace con las primitivas 
(o funciones) especificadas en las normas ISO.


Entorno y lenguaje de programación:
===================================
* Windows XP/VISTA/7.
* Lenguaje de Programación C.
* Biblioteca de simulación del puerto serie que permite enviar y recibir carácter a carácter.


Características:
=================
La aplicación permite la comunicación de dos procesos utilizando los servicios de nivel de enlace. 
Se implementa un protocolo de nivel de enlace orientado a carácter utilizando el código ASCII (CCITT nº 5).
* Un único programa ofrece las funcionalidades de ambos extremos en la comunicación.
* Se muestra en pantalla la primitiva del servicio invocada así como la composición de las tramas.
* Provee de un método de control de errores (checksum o CRC)
* Los nombres de las primitivas y los servicios proporcionados se ajustan al modelo OSI.
* Para mostrar el funcionamiento del servicio se ha implementado una aplicación usuaria del nivel de 
enlace que permite obtener el código Morse de frases cortas.



Uso:
====
* Para lanzar ambos extremos automáticamente, se provee de los scripts correspondientes (morse_1.bat y morse_2.bat).
* La aplicación se provee compilada, pero es posible compilarla de nuevo con el archivo "Makefile".
* También es posible ejecutar directamente la aplicación con el archivo "morse.exe", indicando cada uno de
los extremos (I=izquierdo, D=derecho). Por ejemplo: `morse D`

