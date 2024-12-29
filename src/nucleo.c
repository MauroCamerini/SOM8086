#include "som8086.h"

extern void 	ini_consola	(void);
extern void 	ini_memloc	(void);
extern CODERR 	ini_secbuf	(void);
extern CODERR	ini_interfaz(void);

//
//	INI_NUCLEO
//	La ejecucion del nucleo comienza en inicio.asm, desde alli se llama a esta funcion para pasar
//	el control al codigo escrito en C.
//
void ini_nucleo(void) {	
	// Inicializacion de los modulos.
	ini_consola();
	cescr("SOM8086 cargado con exito!\n");
	
	ini_memloc();
	if(ini_secbuf()) fatal("SECBUF Fallo inicializacion.");
	if(ini_interfaz()) fatal("INTERFAZ Fallo inicializacion.");
	
	fin_memloc();
	
	interfaz();
}


void fatal(const char* msj) {
	
	cescr("ERROR FATAL\n");
	cescr(msj);
	infinito();
}
