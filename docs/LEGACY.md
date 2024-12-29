
#	Sistema Operativo M 8086 (SOM8086)
Proyecto de creacion de un sistema operativo para el procesdador Intel 8086 que funcione desde un disquete 3/4 de 1440kb.
---
*Última actualización enero de 2016*
---
## ESTADO

	
- Programa de arranque.
- Entorno de desarrollo con NASM y OpenWatcom para la generacion de los archivos binarios, WinImage para crear la imagen de disquete, CDR-Tools para crear una imagen de CD que arranque en modo simulacion de unidad de disquete y VMWare para probar el sitema en una maquina virtual.
- Interfaz de usuario minima con escritura en pantalla y lectura desde el teclado de cadenas de
	caracteres.
	
###	9/11
- Funcion `consola.c::conleerx()` que lee desde el teclado aceptando solo los caracteres ASCII
	que se encuentran en un juego de caracteres dado.
- Makefile hecho.

###	10/11
- `itoa()` y `atoi()`
- Lectura y escritura de enteros por consola.
	
###	11/11
- Corregida funcion strchr()
- Echas algunas pruebas para evitar el desalinemiento de los punteros a las cadenas. Si el nucleo tiene un tamano par anda todo bien, sino falla. Probar con la definicion de los	segementos. Probablemente la falla tambien pueda provenir de que la pila es cualquier cosa, se sigue usando hasta ahora la pila que habia definido el programa de arranque.
	
###	13/11
- Se cambio el prefijo *con* por *c* en las funciones del modulo consola para simplificar la 
	lectura y escribir menos.
- Intentando hacer que anden `cleeri()` y `cescri()`. Pero no hubo exito. Parte del problema esta
	en como se define la cedena `digit_alpha` en `misc.c` que usan `itoa()` y `atoi()` para convertir los enteros. Seguimos con los problemas de punteros
- Se agrego definicion de segmentos en `inicio.asm`. El programa no funciona ahora.
	
###	16/11
- Makefile modificado. Ahora crea un archivo `.lnk` para el enlazador en vez de pasar las	directivas por linea de comandos.
- Se estudiaron algunas opciones del compilador Watcom y las definiciones de segmentos para	evitar el problema que tenemos con los punteros.
- El programa de arranque ahora establece `SS = DS`. El desensamblado del nucleo mostro que el codigo a veces hacia algo como esto:
```
lea di, [bp]
mov ax, [di]
```
Ese codigo fallaba porque `BP` es realitvo a `SS` y `DI` relativo a `DS`, pero `SS` y `DS` eran distintos.
Ahora establecimos `SS=DS` para evitar ese problema, y para que el enlazador deje de tirar la advertencia *STACK SEGMENT NOT FOUND* (para eso se retiro la opcion -zu del compilador).
La pila creada por el prorgama de arranque es de 1024 bytes y se encuentra al final del nucleo, en su mismo segemento. En `BX` se pasa al nucleo el puntero a la direccion de memoria inmediatamente posterior a la pila, lo que constituye el comienzo de la memoria a disposicion del nueclo.
	
### 17/11
- Se definio el segmento de pila en `inicio.asm` y ahora si el enlazador deja de tirar la	advertencia *STACK SEGMENT NOT FOUND*. Aparentemente ya no tenemos problemas con los punteros, el programa compila bien.
- `atoi()` arreglada. `cescri()` y `cleeri()` ahora funcionan a la perfeccion.
- `inicio.asm` almacena los datos pasados por el programa de arranque en dos variables globales:
	'unidad' y 'memoria'.
- Modulo `MEMLOC` para el manejo de la memoria local.
- Modulo `DISCO` para lectura/escritura del disco.
- Partes del modulo FAT.
	
### 18/11
- Se modifico el modulo `DISCO` por un mas simplificado `OPDISCO`. Provee una unica funcion llamada	`opdisco()` que es analoga a la interrupcion 0x13 del bios, solamente que a esta se le pasa la direccion absoluta del sector y no la direccion fisica CHS.
- Se creo el modulo BUFSECT que provee el manejo de buferes para sectores alojados en memoria
	local. Se encarga de que este cargado en el bufer el sector solicitado. Si no lo esta, lo carga,
	pero previamente guarda el anterior si fue modificado. Tambien se le puede solicitar que guarde
	un sector. Cargar y guardar quiere decir leer y escribir en el disco. Por lo tanto, los otros
	modulos no necesitan encargarse de eso, solamente deben decir a BUFSECT que sector requieren
	leer o escribir y este se encarga de que el sector este listo en el bufer para realizar esas
	operaciones.
- Modulo `FAT`. Lectura/escritura de clusters en la FAT. Lectura/escritura de entradas del
	directorio raiz.
- Los modulos muestran en pantalla que estan haciendo en operaciones relevantes.
- Se probo la lectura del directorio raiz y funciona.
- Se establecieron codigos de error en `tipos.h` para una comunicacion estandarizada entre las funciones. Tienen la forma `E_<error>`. El codigo `E_EXITO=0` indica que la funcion fue exitosa.
- Se establecieron en tipos.h los valores predefinidos para los campos del Bios Parameter Block.
	Tienen la forma `BPB_<campo>`.
- Se creo el archivo `som8086.h` que contiene todos los achivos de encabezado, aunque algunos	modulos no los requeiren todos, evitamos de esta forma que falte alguno.
	
###	19/11
- Se corrigio la estructura `BUFSECT`, ahora esta mejor disenada. Hay que modificar el codigo	en `bufsect.c` y `fat.c` para que se adapte a esta modificacion.
	
###	20/11
- Rediseno de `BUFSECT`, ahora se llama `SECBUF`. Maneja un numero de buferes para sectores. Los ordena en una lista de acuerdo a su frecuencia de uso y se encarga de cargarlos, guardarlos y liberarlos. Da un tratamiento especial a los sectores correspondientes a la FAT, ya que el	bufer se debe guardar en dos sectores distintos, uno para cada copia de la  FAT.
- Tipo `CODERR`. Es igual a `WORD`, pero indica que la funcion devuelve un codigo de error. Falta	modificar los prototipos de algunas funciones.
- Los prototipos de las funciones de inicializacion de modulos se sacaron de los encabezadores para que no sean accesibles desde todo el programa. Se declara como extern en nucleo.c que es donde se utilizaran.
- Se comenzo la reonversion del modulo FAT para que solo maneje la tabla. El manejo del	directorio raiz se hara aparte.
	
###	23/11
- Trabajando en el manejo de directorios.
	
###	05/12
- Se agregaron las funciones `fatprox()`, `fatborrar()`. La primera devuelve el proximo cluster	en la cadena a partir de un cluster dado. La segunda borra toda una cadena de clusteres.
- Modulo `DIR`, permite abrir un directorio a partir de su ruta, cargar y guardar entradas. Nos	falta algo fundamental, la funcion davan() que permite moverse dentro del directorio.
- Estructura `FICH` que contiene la informacion de un archivo o directorio abierto. Basicamente	contiene su entrada de directorio (y ubicacion en el disco de la misma para poder modificarlo),	la posicion de lectura/escritura dentro del archivo y cluster al que corresponde dicha posicion.
- Funcion `davan()` que avanza la posicion dentro del directorio.
	
### 21/12
- Luego de mucho tiempo de inactividad, releimos el codigo en `dir.c` y `secbuf.c`. Se corrigio algun que otro error y se agregaron comentarios.
- Se eliminaron el sinnumero de archivos .h. Ahora tenemos `general.h`, `som8086.h` y `fat.h`.
	- El primero reune los viejos tipos.h y misc.h, funciones y tipos de uso general. 
	- El segundo incluye al primero y agrega todas las funciones y tipos de los modulos.
	- El ultimo corresponde	al modulo fat, que como era mas extenso que el resto se dejo en un archivo aparte. Cualquier archivo .c debe solamente incluir som8086.h.
- Se implemento la funcion `dnueva()`, que posiciona al directorio en la primer entrada libre, expandiendo el directorio si es necesario.
	
## TAREAS
	
- No manejo de subdirectorios, solamente el directorio raiz.
- Tenemos que tener un interprete de linea de comandos basico para poder ir probando las cosas
- Las funciones anteriores son condicion para poder avanzar en tareas como crear, borrar, leer y escribir archivos.
- Algun bufer para estructuras FICH permanentes. Alguna forma de registrar los archivos abiertos para que no se puedan abrir dos veces y no se los pueda eliminar o cambiar de lugar una vez que estan abiertos.
- Reacomodar un poco los archivos .h. Hay uno por cada archivo .c pero luego en la practica, los modulos incluyen todos los .h ya que incluyen a som8086.h. Hay muchas cosas diversas	en tipos.h y misc.h, a veces no se donde buscarlas porque pueden estar en uno o en otro. misc.h	tiene un monton de cosas que no son relativas a misc.c. Me parece una buena practica tener un .h por cada .c porque es mas ordenado, pero se me hace un matete con las definiciones que son	generales y especificas. Abria que reformar tipos.h y misc.h para unificarlos en un solo archivo generico. Despues ver bien este tema.