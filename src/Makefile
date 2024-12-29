#
# Procedimiento:
#
# Los archivos fuente en ensamblador, de extension .asm, se compilan con NASM.
# Los archivos escrito en C, de extension .c, se compilan con OpenWatcom.
# Ambos programas almacenaran los archivos objeto (.obj) en la carpeta obj. Se agergara una
# referencia a eso archivos en $(TARGET).lnk.
# En enlazador WLINK es el utilizado para producir el archivo binario a partir de todos los
# archivos OBJ. Produce un archivo binario plano y lo almacena en el directior BIN.
# Correr limpiar.bat antes de ejecutar WMAKE.


#
# Programas requeridos para compilar el nucleo: OpenWatcom WCC, NASM y WLINK
#
CC=wcc
WLINK=wlink
NASM=nasm

# Punto de inicio del codigo del nucleo.
START=inicio

# Nombre del archivo que se esta creando
TARGET=nucleo

# Archivo binario final
BIN=../bin/$(TARGET).bin

# Archivos objetos que es necesario procesar. Cuando se agregue un nuevo archivo fuente al codigo
# agregarlo a esta lista al igual que el resto que ya fueron agregados.
OBJ=../obj/inicio.obj ../obj/nucleo.obj ../obj/consola.obj ../obj/misc.obj ../obj/memloc.obj ../obj/opdisco.obj ../obj/secbuf.obj ../obj/fat.obj 

# Opciones para NASM
NFLAGS=-f obj
#
#	-f		Formato utilizado: OBJ.

# Opciones para OpenWatcom WCC
CFLAGS=-i=h -0-d0-s-wx-zl-ms 
#
#	-0     	Instrucciones para el 8086
#	(no utilizada)-od		Deshabilita todas las optimizaciones
#	-d0		No informaccion de debugging
#	-ms		Modelo de memoria "small". CS = ES = DS
#	-s		No hay verificacion de sobrecarga de la pila
#	-wx		Nivel de advertencias al maximo
#	-zl		No genera referencias a las librerias en el archivo objero.
#	(no utilizada)-zu		No asume que SS es igual a DS
#	-i		Directorio donde se buscan los archivo .h

# Opciones pasadas a WLINK
#
#	NAME			Nombre del archivo resultado del proceso de enlazado
#	FORMAT RAW BIN	Se creara un archivo binario plano
#	OPTION
#	* NODEFAULTLIBS	No agrega ninguna libreria estandar
#	* START			Donde se encuentra el punto de incio del codigo
$(BIN): $(OBJ)
	echo NAME $(BIN) >> $(TARGET).lnk
	echo FORMAT RAW BIN >> $(TARGET).lnk
	echo OPTION NODEFAULTLIBS >> $(TARGET).lnk
	echo OPTION START=inicio >> $(TARGET).lnk
	$(WLINK) @$(TARGET).lnk

	
.c.obj:
	$(CC) $(CFLAGS) -fo=$*.obj $<
	echo FILE $*.obj >> $(TARGET).lnk
	
.asm.obj:	
	$(NASM) -o $*.obj $(NFLAGS) $<
	echo FILE $*.obj >> $(TARGET).lnk