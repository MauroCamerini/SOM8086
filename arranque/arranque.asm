;
; ARRANQUE
;
; Este programa debe ubicarse en el sector 0 del disquete. Cuando la computadora se enciende, el
; BIOS carga dicho sector en la direccion de memoria 0000:7C00 y le pasa el control.
; La tarea de este programa cuando obtiene el control de la computadora, es cargar el resto del
; sistema operativo desde el disco, y luego pasarle el control al mismo.
;
; Este programa de arranque tiene los siguientes requisitos:
; * Estar instalado en una unidad de disquete de 1440kb formateado con el sistema de archivos
; FAT12.
; * La entrada en el directorio raiz del archivo que contiene al sistema operativo debe ser la
; primera.
; * Dicho archivo debe encontrarse almacenado en el disco en el comienzo del area de datos de
; forma continua.
;
; La consecuencia de estos requisitos es que el archivo que este programa de arranque debe cargar
; se encuentra alamcenado en el disco en una ubicacion preestablecida, por lo tanto no debe buscarlo
; porque ya sabe donde esta. Si alguno de estos requisitos no se cumple el programa de arranque 
; fallara.
;
; Para entender esto ultimo veamos la estructura interna del disco formateado con el sistema de
; archivos FAT12.
;
; SECTOR 0 		- Donde se ubica este programa de arranque.
; SECTOR 1-9	- FAT (File Allocation Table), ocupa 9 sectores.
;				  A partir del cluster 2 se encuentran la info de los clusteres del sistema operat.
; SECTOR 9-18	- Copia de la FAT, el sistema posee una copia de la FAT por seguridad.
; SECTOR 19-32	- Directorio raiz.
;				  Los primeros 32 bytes constituyen al entrada del archivo que se va a cargar.
; SECTOR 33...  - Area de datos, donde se encuentra la informacion del archivo.
;				  El archivo que se desea cargar se encuetra en esta ubicacion.
;

; La computadora al encenderse, sin importar que procesador tenga, comienza funcionando como si
; tuviese un 8086 de 16-bits.
	bits	16
	cpu		8086

; Este programa se carga en la direccion 0000:7C000.
; Al comenzar el programa BP es establecido con este valor, por lo tanto las direcciones de
; memoria pueden obtenerse como desplazamientos con respecto a BP, lo que resulta en instrucciones
; que ocupan menos espacio.
%define	BASE 0x7c00		

%define BYTES_P_SECTOR  bp+0x0b ; bytes/sector
%define UNIDAD 	bp+0x24  	; Numero de unidad (en el BIOS PARAMETER BLOCK)

%define ENTRADA	bp-0x7700	; Bufer para la primera entrada del dir raiz (0000:0500)
%define CLUSTER bp-0x76E6	; Primer cluster del archivo (info en la entrada de dir)
%define TAMANO0	bp-0x76E4	; Tamano en bytes del archov (info en la entrada de dir)
%define TAMANO1	bp-0x76E2	; Tamano en bytes del archov (info en la entrada de dir)
%define NUCLEO  bp-0x7500	; Ubicacion donde se debe cargar el archivo (0000:0700)
; Tener en cuenta que el archivo a cargarse no puede tener mas de 29kb, sino no entra en el espacio
; de memoria que se le asigno que es entre la dir 0x700 y la 0x7C00 que es donde se ubica este
; programa.

; Ubicaciones dentro de la unidad de sectores clave.
%define INI_RAIZ  19	; Directorio raiz, 21 = 1(arranque) + 2 * 9(fat)
%define INI_DATOS 33 	; Area de datos 33 = 1(arranque) + 2 * 9(fat) + 14(dir raiz)


segment	.text
	org		BASE

; Este salto evita que se ejecuten los datos que se encuentra a continuacion
		jmp		short inicio
		nop
		
; BIOS PARAMETER BLOCK
;
; Esta seccion contiene informacion sobre la unidad. Los datos, prefijados de esta manera, son
; coincidentes con el tipo de disquete en el que el programa de arranque espera estar instalado.
; No modificarlos, y especialmente, no instalar este programa en una unidad que no cumpla estas
; caracteristicas porque no funcionara.
		db 'SO M8086'   ; 8bytes, nombre del sistema operativo
		dw 512          ; Tamano en bytes de cada sector del disco
		db 1            ; Cantidad de sectores que ocupa un cluster del sistema de archivos
		dw 1            ; Sectores reservados. Uno, solo este.
		db 2            ; Numero de copias de la FAT.
		dw 224          ; Numero de entradas en el directorio raiz, permite inferir su tamano tamb.
		dw 2880         ; Cantidad total de sectores en el disco
		db 0xF0         ; Indica el tipo de unidad
		dw 9            ; Cantidad de sectores que ocupa cada FAT
		dw 18           ; Numero de sectores por cada cilindro fisico del disco.
		dw 2            ; Cantidad de cabezales
		dd 0            ; Sectores ocultos. Ninguno.
		dd 0            ; Numero total de sectores en 32-bits, utilizado por unidades mas grandes.
		db 0            ; Numero de unidad (sera modificado en tiempo de ejecucion)
		db 0            ; Reservado
		db 0x29         ; Indica que esta disponible la siguiente informacion
		dd 0x12141618   ; Numero de serie del disco
		db 'SO M 8086  '; 11 bytes, etiqueta de la unidad
		db 'FAT12   '   ; 8 bytes, nombre del sistema de archivos.


; *************************************************************************************************		
; INICIO
;
; Aqui comienza la ejecucion del progrma de arranque, proveniente desde el salto ubicado mas arriba.
; 
; *************************************************************************************************

inicio:
		cli				; Desabilita las interrupciones hasta que inicializa todos los registros
		cld				; Las operaciones de cadena van en direccion ascendente
		
		xor		ax, ax	; Todos los registros de segemento = 0
		mov     ds, ax  
		mov     es, ax
		mov     ss, ax
		
		mov     bp, BASE
		lea     sp, [bp-4] ; Establece la pila inmeditamente antes de donde se cargo este programa
		
		sti				; Habilita nuevamente las interrupciones
		
		mov		[UNIDAD], dl ; El BIOS pasa el numero de unidad en DL
				
		;
		; Carga la entrada del dir raiz, contenida en el primer sector del mismo.
		;
		mov		ax, INI_RAIZ
		lea		bx, [ENTRADA]
		call	leer_sector
		
		
		;
		; Verifica que el nombre de archivo coincida con el que buscamos
		;
		lea     si, [ENTRADA]
		mov		di, nombre_archivo
		mov		cx, 11				; 11 caracteres, 8 para el nombre y 3 para la extension
		repe	cmpsb				; Comparacion
		je		es_el_archivo		
		
		call	error_fatal ; Si no coincide, mensaje de error y nos vimos en disney
		db		'Archivo no coincide',0
		
es_el_archivo:


		;
		; Verifica que el archivo se encuentre donde esperamos (al inicio del area de datos)
		;
		mov		ax, [CLUSTER]
		cmp		ax, 2
		je		esta_donde_queremos
		
		call	error_fatal
		db		'Nucleo mal ubicado',0
		
esta_donde_queremos:
		; Calcula la cantidad de sectores que ocupa el archivo
		mov		ax, [TAMANO0]
		mov		dx, [TAMANO1]
		mov		cx, 512		; CX = BYTES_P_SECTOR
		div		cx
		; AX = cociente de TAMANO / BYTES_P_SECTOR
		; DX = resto de TAMANO / BYTES_P_SECTOR
		; Si quedo un resto, hay que sumarle uno a AX, sino ya estamos
		test	dx, dx
		jz		short cargar
    	inc		ax
		jmp		short cargar
		
		
		
		;
		; Carga del archivo
		;
		; Del codigo anterior, tenemos  en AX la cantidad de sectores que hay que cargar
		; a partir del comienzo del area de datos de la unidad.
		; Como se explico mas arriba, se espera que todos los sectores del archivo esten guardados
		; consecutivamente en el disco. Hemos corroborado que el primer sector este donde debe estar
		; pero no los otros. En caso de que el archivo este fragmentado dentro del disco el
		; comportamiento es impredecible. Estar muy atento a esto en el proceso de creacion del
		; disco de arranque.
		; Otra cosa que se debe tener en cuenta es que el archivo se carga en la direccion 0x700
		; y le programa de arranque se encuentra en la 0x7C00. Por lo tanto el archivo no puede
		; superar los 29kb, sino no entra y emepzaremos a sobreescribir el propio codigo de este
		; programa, lo cual tiene un comportamiento impredecible.
		;
				
sector			dw 0			; Numero de sector que se va a cargar
puntero			dw 0			; Puntero donde se almacenara el sector

cargar:
		test	ax, ax
		jnz		short noincr
		inc		ax
		
noincr:	mov		cx, ax ; CX = cantidad de sectores que hay que leer
		
				
		; Inicializa las variables	
		mov		[sector], word INI_DATOS ; sector = primer sector del area de datos
		lea		bx, [NUCLEO]
		mov		[puntero], bx		; puntero = 0700, offset de la direccion donde se carga el arch

proximo:
		push	cx		; Preserva el contador
		
		; Carga el sector
		mov		ax, [sector]
		mov		bx, [puntero]
		call	leer_sector
		
		; Actualiza las variables para la proxima iteracion
		add		[sector], word 1
		add		[puntero], word 512
		
		pop		cx		; recupera el contador
		loop	proximo ; si CX <> 0 continuara

		; +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
		;
		; ARCHIVO CARGADO !!!
		;
		; Llegando a esta parte ya cargamos todo el archivo en memoria a partir de la direccion
		; 0070:0000
		;		
				
		cli
		
		; Pasa en DL el numero de unidad.
		mov		dl, [UNIDAD]
		
		; Crea una pila de 1024 bytes.
		; Pasa al nucleo en CX el puntero a la primera direccion de memoria libre posterior a si
		; mismo.
		mov		ax, [puntero]
		sub		ax, 0x300	; Le suma 1024 (0x400, el tamano de la pila), le resta 0x700, el offset
							; donde se carga al nucleo (=-0x300). Esto es importante porque el 
							; programa de arranque usaba los segmentos en 0000 pero el nucleo en
							; 0070, por lo tanto sin esta correccion el puntero no tendra la
							; direccion correcta cuando el nucleo acceda en la forma DS:puntero.
		
		; Pase en BX el puntero a la memoria libre
		mov		bx, ax		
		
		mov		sp,	ax		; Establece SP.
		dec		sp
		dec		sp
		
		; Todos los segmentos = 0x0070. Esto es asi porque el nuecleo usa un modelo de memoria SMALL
		; y esta enlazado para un OFFSET = 0.
		mov		ax, 0x0070
		mov		ds, ax
		mov		es, ax
		mov		ss, ax

		sti
		
		;
		; HECHO. PASAMOS EL CONTROL AL NUCLEO.
		;
		jmp		0x0070:0000
		
; *************************************************************************************************
; ERROR FATAL
;
; Muestra un mensaje de error y entra en un bucle infinito.
;
; La cadena que contiene el mensaje debe estar inmediatamente despues de la instruccion CALL que
; llamo a esta funcion. Como dicha instruccion inserta en la pila la direccion de la instruccion
; posterior a si misma, y como alli hemos puesto una cadena de caracteres, la instruccion CALL lo
; que termina haciendo es insertando en la pila la direccion al primer caracater de la cadena.
; La cadena debe terminar con un caracter nulo
;
; *************************************************************************************************

error_fatal:
				pop     si			; Obtiene el puntero a la cadena
				
bucle_fatal:    lodsb               ; Carga el caracter apuntado por SI e incrementa SI.
				test    al, al		; Es el carcter nulo?
                jz      short infinito 	; Entonces termina.
				
                mov     ah,0x0E     ; Funcion 0x0E. "Imprimir un caracter en pantalla".
                int     0x10		; Interrupcion 0x10 del BIOS. Muestra el caracter.
                jmp     short bucle_fatal
				
infinito:		jmp		$			; Bucle infinito

; *************************************************************************************************				
; LEER_SECTOR
;
; Carga un sector en una direccion especifica en memoria
;
; Argumentos:
; 	AX = Numero de sector lógico
; 	ES:BX = Puntero al bufer donde se cargara el sector
;
; Esta función recibe en AX el número de sector logico. Dicho modo de direccionamiento asume que
; los sectores son contiguos y estan numerados del 0 al n. Lamentablemente esa no es la forma
; en la que realmente se accede a los sectores del disquete. Para hacerlo hay que pasarle al BIOS
; el numero de cilindro, cabezal y sector físico donde se encuentra la información que queremos
; obtener, por sus siglas en ingles CHS (Cylinder, Head, Sector). C y H comienzan en 0, pero el
; primer sector físico siempre es 1, esto se debe tener en cuenta.
; Por lo tanto se requiere traducir la dirección lógica a una dirección CHS. Lo cual se hace con
; la siguiente ecuación:
;
; 	sect_logico = C * sect_p_cilndro * n_cabezales + H * sect_p_cilindro + S - 1
;
; 	sect_logico			lo tenemos en AX
; 	sect_p_cilindro		Sectores por cilindro, lo sabemos: 18
; 	n_cabezales			Numero de cabezales, lo sabemos: 2
; 	C, H, S				Cilindro, cabezal y sector físco. Lo que queremos averiguar.
;
; Para conseguir la dirección CHS debemos hacer lo siguiente:
;
;	temp = sect_logico DIV sect_p_cilindro = sect_logico DIV 18
;	C = temp DIV n_cabezales = temp DIV 2
;	H = temp MOD n_cabezales = temp MOD 2
;   S = (sect_logico MOD sect_p_cilindro) + 1 = (sect_logico MOD 18) + 1
;
;	a DIV b = cociente de a/b, a MOD b = resto de a/b
;
; *************************************************************************************************
leer_sector:
							; AX = sect_logico (pasado como argumento)
		mov		cx, 18		; CX = sect_p_cilindro = 18
		
		xor		dx, dx
		div		cx			; AX / CX = sect_logico / sect_p_cilindro
		; AX = sect_logico DIV sect_p_cilindro 
		; DX = sect_logico MOD sect_p_cilindro
		
		inc		dx
		; AX = temp
		; DL = S
				
		mov		cl, 2		; CL = n_cabezales = 2
		div		cl		    ; temp / n_cabezales
		; AL = C = temp DIV n_cabezales
		; AH = H = temp MOD n_cabezales
		
		; Obtuvimos la ubicacion fisica del sector. Imporante: esta cuenta funciona solamente 
		; porque estamos en un disco chico, en discos mas grandes C tendria mas de 8bits y no 
		; caberia en AL.
		
		; AL = C, AH = H, DL = S
		
		; Ahora hay acomodar esa informacion tal como lo requiere el BIOS
		; DH = H
		; CH = C
		; CL = S
		mov		dh, ah			; DH = H. Numero de cabezal
		mov		ch, al			; CH = C. Numero de cilindro
		mov		cl, dl			; CL = S. Numero de sector
		mov		dl, [UNIDAD]	; DL = Numero de unidad. Lo obtuvimos al comienzo.
		mov		ax, 0x0201		; AH = funcion 0x02. "Leer sectores del disco"
								; AL = 1. Lee un solo sector
								; ES:BX = Puntero al bufer. Se paso como argumento.
		int		0x13			; Interrupcion 0x13 del BIOS. Lee el sector
		
;		test	al, al			; AL = 0?
;		jz		short hecho		; si, entonces todo bien.
;		call	error_fatal		; no, entonces hubo algun error, fallo el proceso de arranque.
;		db		'Error al leer el disco',0
		
hecho:	ret
		
; *************************************************************************************************
; FIN DEL CODIGO
; *************************************************************************************************
		
		; Completa con 0 para obtener un archivo de 512 bytes
		times   0x01F3-$+$$ db 0
		
		; Nombre del archivo que se desea cargar. Debe dejarse en esta ubicacion fija para poder
		; ser accesible por pogramas que quieran modificar este sector de arranque.
		nombre_archivo:	db	'NUCLEO  BIN'	
		
        ; Esta marca le indica al BIOS que es un disco ejecutable, es decir, que en este sector
		; se encuentra un programa de arranque.
		dw      0xAA55
		