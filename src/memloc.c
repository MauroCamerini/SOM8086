#include "som8086.h"

//
//  MEMLOC.C
//	Manejo de la memoria local.
//
//  Lo que hace este modulo es otorgarle al resto de los modulos que lo soliciten una porcion
//	de la memoria que se encuentra luego de la pila del nucleo.
//	Aunque esta memoria esta asignada dinamicamente, no se permite liberarla ni reacomodarla.
//
//	ESTRUCTURA DE LA MEMORIA
//
//  Direccion	Contenido		Largo en bytes
//	------------------------------------------
//	0070:0000	NUCLEO			x bytes
//	fin nucleo	PILA			1024 bytes
//	inilocal	MEMORIA LOCAL	largomemlocal
// 	finlocal	...				...
//

// Definida en inicio.asm. Contiene la posicion de memoria inmediatamente posterior al final
// de la pila.
extern BYTE* memoria;

// Puntero al comienzo de la memoria local.
static BYTE* inilocal;

// Puntero al final de la memoria local. Posicion del proximo bufer.
static BYTE* finlocal;

// Cantidad de bytes reservados en la memoria local.
static WORD largomemlocal;

// Determina si puede reservar memoria local o no;
static BOOL abierto = FALSE;

//
//	INI_MEMLOC
//	Inicializa el modulo de manejo de memoria local.
//
//	memoria		Puntero al comienzo de la memoria local.
//
void ini_memloc(void) {
	abierto = TRUE;					// Habilita el uso de la funcion reslocal()
	inilocal = finlocal = memoria;	// Inicializa las variables.
	largomemlocal = 0;				// No se reservo nada por ahora.
	
	cescr("MEMLOC Inicializado.\n");
}

//
//	RESLOCAL
//	Reserva una porcion de memoria para uso local. Esta memoria no se puede liberar.
//
//	sz		Largo de la porcion que se desea reservar.
//
//	DEVUELVE: El puntero a la memoria local reservada o NULL si hubo algun error.
//
BYTE* reslocal(WORD sz) {

	WORD relleno;
	BYTE* ptr = finlocal; // Preserva el puntero que se debe devolver.
	
	if(abierto == FALSE) return NULL;
	cescr("MEMLOC Asignando bufer de ");
	cescri(sz, 10);
	cescr(" bytes.\n");
	
	// Alinea la memoria a posiciones multiplo de 16.
	if((relleno=sz%16) > 0)
		sz += 16 - relleno;
	
	largomemlocal+=sz;		// Obtiene el nuevo largo de la memoria local
	finlocal+=sz;			// Actualiza el puntero al final de la memoria local
	
	return ptr;
}

//
//	SZMEMLOC
//	Devuelve el tamano de la memoria reservada para uso local.
//
//	DEVUELVE: EL tamano de la memoria resevda para uso local.
//
WORD szmemloc(void) {
	return largomemlocal;
}

//
//	FIN_MEMLOC
//	Imposibilita el uso de la funcion reslocal(). Devuelve el puntero a la memoria que se encuentra
//	luego de la memoria local.
//
//	DEVUELVE: El puntero a la memoria que se encuentra luego de la memoria local.
//
BYTE* fin_memloc(void) {
	abierto = FALSE;
	return finlocal;
}
