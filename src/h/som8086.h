
#ifndef SOM8086_H
#define SOM8086_H

#include "general.h"


//
//	CONSOLA  **************************************************************************************
//
//	Funciones para la lectura desde el teclado y la escritura en pantalla.
//

extern void 	nuevalinea	(void);
extern char* 	cleerx		(char*, INT16, const char*);
#define 		cleer(a,b) 	cleerx(a,b, NULL)
extern void 	cescr		(const char*);
extern char 	cleercx		(const char*);
#define			cleerc() 	cleercx(NULL)
extern void 	cescrc		(char);
INT 			cleeri		(void);
void 			cescri		(INT n, INT base);

//
//  MEMLOC  ***************************************************************************************
//
//	Manejo de la memoria local.
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


extern BYTE* 	reslocal	(WORD);
extern WORD 	szmemloc	(void);
extern BYTE*	fin_memloc	(void);

// 	OPDISCO  **************************************************************************************

// Codigos de error especificos del modulo DISCO
#define EDSC_CMDINVAL	0x01FF // bad command: invalid request to controller
#define EDSC_DIRINVAL	0x02FF // bad address mark
#define EDSC_SOLOLECT	0x03FF // write protect: attempted to write on write-protected diskette
#define EDSC_SECINVAL	0x04FF // sector ID bad or not found
#define EDSC_CAMBIADO	0x06FF // diskette change line is active
#define EDSC_DMAFAIL	0x08FF // DMA failure
#define EDSC_DMAOVERR	0x09FF // DMA overrun: attempted to write across a 64K-byte boundary.
#define EDSC_MEDINVAL	0x0cFF // Media type not available
#define EDSC_BADCRC		0x10FF // bad CRC: Cyclical Redundancy Code does not agree with data
#define EDSC_CONTRERR	0x20FF // diskette controller failure
#define EDSC_CININVAL	0x40FF // bad seek; requested track not found
#define EDSC_FUETIEMPO	0x80FF // time-out

// Operaciones posibles con operdisco
#define DSC_REINI 	0
#define DSC_ESTADO 	1
#define DSC_LEER 	2
#define DSC_ESCR 	3

extern CODERR opdisco(WORD op, WORD sect, BYTE* buf);

// Cantidad de buferes que se reserva 
#define SEC_N_SECBUF 4

struct struct_secbuf {
	struct struct_secbuf* prox; // Bufer siguiente
	struct struct_secbuf* prev; // Bufer anterior
	WORD flags;					 // Flags
	WORD sect;					 // Sector cargado en el bufer
	BYTE relleno[8];			 // Se pone un relleno para el el bufer se encuentre en una posicion
								 // de memoria multiplo de 16.
	BYTE buf[BPB_LARGO_SECT];

};

//	SECBUF  ***************************************************************************************

typedef struct struct_secbuf SECBUF;

#define SEC_VACIO		0
#define SEC_OCUPADO	  	0x01 // Posee un sector cargado
#define SEC_MODIF	  	0x02 // La informacion fue modificada
#define SEC_FAT	  		0x10 // Sector de la FAT
#define SEC_DRAIZ	  	0x20 // Sector del directorio riaz
#define SEC_DATOS		0x40 // Sector del area de datos

extern SECBUF* 	sec_obtener		(WORD);
extern CODERR 	sec_guardar		(SECBUF* bp);
extern CODERR	sec_guardtodos	(void);
extern void 	sec_vaciartodos	(void);

//	FAT  ******************************************************************************************

#include "fat.h"

//	FICH  *****************************************************************************************

//
//	Contiene al informacion de un archivo o directorio abierto para lectura y/o escritura.
//
struct struct_fich {
	WORD 	flags;		// Indicadores
	
	// Codigo de error de la ultima operacion sobre el fichero.
	//CODERR  coderr;
	
	WORD 	pos;		// Posicion absoluta dentro del archivo
	WORD 	clustn;		// Numero de cluster actual realitvo al archivo
	
	WORD 	cluster;		// Cluster actual en la fat.
	
	DIRENT 	dirent;		// Entrada de directorio del archivo
	
	// Esto es neceario por si se necesita acutalizar la informacion dentro de la entrada
	WORD	entsect;	// Sector donde se encuentra la entrada
	WORD	entdesp;	// Desplazamiento dentro del sector
}

typedef struct struct_fich FICH;

#define F_LIBRE		0
#define F_RAIZ		0x08	// Es el directorio raiz, usarlo junto con F_DIR
#define F_DIR		0x10	// Es un directorio, puede tener al mismo tiempo F_RAIZ

//	DIR  ******************************************************************************************

extern CODERR 	dabrir	(const char* ruta, FICH* fp, BOOL archivo);
extern CODERR 	davan	(FICH* fp);
extern void 	drebo	(FICH* fp);
extern CODERR 	dleer	(FICH* fp, DIRENT** entpp);
extern CODERR 	descr	(FICH* fp, DIRENT* entp);
extern CODERR	dnueva	(FICH* fp);

#endif
