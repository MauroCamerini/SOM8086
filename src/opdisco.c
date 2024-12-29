#include "som8086.h"

// Numero de unidad. Almacenado en inicio.asm y obtenido del programa de arranque.
extern BYTE unidad;


//	Interrupcion del bios 13h. Operacions con el disco.
BYTE bios_int13(BYTE fn, BYTE unidad, BYTE c, BYTE h, BYTE s, BYTE* buf);
#pragma aux bios_int13 = \
		"mov	al, 1" \
		"int	13h" \
		"jc		n" \
		"xor	ah, ah" \
		"n:		nop" \
		modify	[AL] \
		parm 	[AH] [DL] [CH] [DH] [CL] [BX] \
		value	[AH]

	
//
//	OPDISCO
//	Realiza una operacion sobre el disco utilizando la interrupcion 0x13 del BIOS.
//	Operaciones:
//	DSC_REINI	Reinicia los controladores de disco. Se lo debe llamar luego de que fracase una
//		operacion de lectura escritura.
//	DSC_ESTADO 	Devuelve el codigo de error especifico de la ultima operacion de disco.
//	DSC_LEER	Lee el sector requerido y lo guarda en el bufer.
//	DSC_ESCR	Escribe el contenido del bufer en el sector expecificado.
//
//	op		Codigo de operacion, determina distintos tipos de funcionamiento.
//	sect	Numero de sector que se quiere leer/escribir.
//	buf		Bufer donde se leera/escribira el sector.
//
//	DEVUELVE: Codigos de error especificos
//
CODERR opdisco(WORD op, WORD sect, BYTE* buf) {
	
	BYTE c,h,s,r;

	if(op > DSC_LEER) return E_ARGINVAL;
	
	// Calcula la ubicacion fisica del sector en el disco para operaciones de lectura/escritura
	if(op == DSC_LEER || op == DSC_ESCR) {
		WORD tmp;
		tmp = sect / BPB_SECT_P_CIL; 
		c = (BYTE)(tmp / BPB_N_CABEZALES); 
		h = (BYTE)(tmp % BPB_N_CABEZALES); 
		s = (BYTE)(sect % BPB_SECT_P_CIL + 1);
	}
	
	r = bios_int13(op, unidad, c, h,s, buf);
	if(r) 
		return MKWORD(r, E_ESPECIF);
	else
		return E_EXITO;
	
}
