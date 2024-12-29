cpu		8086

group   DGROUP _TEXT _DATA CONST CONST2 _STACK

segment _TEXT 	class=CODE use16
segment _DATA	class=DATA align=2
segment CONST	class=DATA align=2
segment CONST2 	class=DATA align=2
segment _STACK	class=STACK align=2

segment _TEXT

; Funcion inicial del nucleo en C
extern ini_nucleo_

; *************************************************************************************************
; INICIO
;
; Aqui es donde la historia comienza. El programa de arranque carga el nucleo en la direccion
; 0070:0000, estableciendo CS = DS = ES = SS = 0070, y salta a esta ubicacion.
; 
; DL	Numero de unidad.
; BX	Puntero a la primera porcion de memoria libre posterior al nucleo..
; *************************************************************************************************
global inicio
inicio:

	; Alamacena la informacion pasada por el programa de arranque
	mov		[_unidad], dl
	mov		[_memoria], bx
	call	ini_nucleo_
	
global infinito_	
infinito_:	jmp		$
	
	; Para encontrar una funcion rapidamente al desensamblar
	;extern	funcion_
	;call	funcion_
	
segment CONST

; Informacion pasada por el programa de arranque.
global _unidad
_unidad	db	0
global _memoria
_memoria dw	0
