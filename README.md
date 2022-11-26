# Implementación de una aplicación de chat sencilla utilizando sockets en C

Desarrollado por Donato Galo.
correo: donatojgalo@gmail.com

Este programa está escrito en el lenguaje C.
Se utilizan sockets TCP para las conexiones.

Para compilar se puede usar el Makefile.
Si desea compilarlo de otro modo recuerde incluir la librería pthread.

Para ejecutarlo se debe correr el servidor para que acepte las conexiones de los clientes.
Tanto la dirección como el puerto de conexión se definen en el archivo globals.h.
Modificando dichos valores se puede configurar la conexión.

Archivos:
* `chatServer.c`: implementación del servidor de la aplicación.
* `chatClient.c`: implementación del cliente de la aplicación.
* `globals.h`: variables y definiciones globales de la aplicación.
* `Makefile`: archivo de compilación.
