
#ifndef GENERAL_H
#define	GENERAL_H

//
// 	GENERAL.H
//	Tipos, macros y funciones generales. 
//	Se definien los tipos enteros y de punteros con sus respectivas macros para manejarlos.
//	Se definen funciones que copian a las de la libreria estandar C
//

// Tipos enteros (para el compilador OpenWatcom short=int=16bits)
typedef unsigned char	BYTE;
typedef unsigned short  WORD;
typedef unsigned long   DWORD;

typedef char    INT8;
typedef short   INT16;
typedef long 	INT32;

typedef short 		   INT;
typedef unsigned short UINT;
typedef long 		   LONG;
typedef unsigned long  ULONG;

// No es mas que una WORD, pero asi podemos ver que funciones entregan codigos de error.
typedef unsigned short CODERR;

// Devuelven respecticamente los BYTE menos y mas significativos de la WORD dada.
#define LOBYTE(w) ((BYTE)(w))
#define HIBYTE(w) ((BYTE)((w)>>8u))

// Crea una WORD a partir de dos BYTE.
#define MKWORD(h,l) (((WORD)(h))<< 8u | ((BYTE)(l)))

// Tipo al que se puede acceder alternadamente como WORD o como dos BYTE
typedef union {
	WORD word;
	struct { BYTE lo, hi; } byte;
} WORDHL;

// Tipo booleano
typedef short BOOL;
#define TRUE	1
#define FALSE	0

// Puntero nulo.
#define NULL 0

// Codigos de error generales
#define E_EXITO 	0	 // Sin error
#define E_ERROR 	1	 // Error generico, sin especficar
#define E_ERRDISCO	2	 // Error de lectura escritura del disco.
#define E_ARGINVAL	3	 // Se paso a la funcion un argumento incorrecto.
#define E_RESMEM	4	 // Se intnento reservar memoria y no se pudo.
#define E_FATCOPIA  0x10 // No se pudo guardar la segunda copia de la FAT
#define E_FINDIR	0x11 // Se alcanzo el final del directorio
#define E_DSCLLENO  0x12 // El disco esta completo.
#define E_ESPECIF	0xFF // Codigo de error especifico de la funcion o el modulo en el otro byte.

// Crea un puntero FAR (segmento:offset).
#define MK_FP(seg,ofs) 	      (((WORD)(seg)):>((void *)(ofs)))

// Crea un puntero FAR del tipo dado.
#define MK_PTR(type,seg,ofs) ((type FAR*) MK_FP (seg, ofs))

// Lectura y escritura de direcciones de memoria FAR.
#define pokeb(seg, ofs, b) (*((unsigned char far *)MK_FP(seg,ofs)) = b)
#define poke(seg, ofs, w) (*((unsigned far *)MK_FP(seg,ofs)) = w)
#define pokew poke
#define pokel(seg, ofs, l) (*((unsigned long far *)MK_FP(seg,ofs)) = l)
#define peekb(seg, ofs) (*((unsigned char far *)MK_FP(seg,ofs)))
#define peek(seg, ofs) (*((unsigned far *)MK_FP(seg,ofs)))
#define peekw peek
#define peekl(seg, ofs) (*((unsigned long far *)MK_FP(seg,ofs)))

// Devuelven respectivamente el segmento y el offser de un puntero FAR
#define FP_SEG(fp)            ((unsigned)((DWORD)(void far *)(fp)>>16))
#define FP_OFF(fp)            ((unsigned)(fp))

//
// MISC  ******************************************************************************************
//
// Miscelaneo
// Implementacion de algunas funciones de la libreria estandar de C.
//

//
//	Entero a ASCII y viceversa.
//
#define BUFER_ITOA	18	// 18 = 16(digitos binarios como max.) + 1(signo -) + 1(caracter nulo)
extern char* 	itoa	(INT n, char* p, INT base);
extern INT  	atoi	(const char* p, INT base);

//
//	Evaluacion de caracteres ASCII
//
#define islower(c) (c >= 'a' && c <= 'z')
#define isupper(c) (c >= 'A' && c <= 'Z')
#define isalpha(c) (islower(c) || isupper(c))
#define isdigit(c) (c >= '0' && c <= '9')
#define isxdigit(c) ( (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F') || isdigit(c))
#define iscntrl(c) ((c >= 0 && c <= 0x1f) || c == 0x7f)
#define isalnum(c) (isalpha(c) || isdigit(c))
#define isprint(c) (!iscntrl(c))
#define isgraph(c) (isprint(c) && c != ' ')
#define ispunct(c) (isgraph(c) && !isalnum(c))
#define isspace(c) ((c >= '\t' && c <= '\r') || c == ' ')

#define tolower(c) (isupper(c) ? c - ('A' - 'a') : c)
#define toupper(c) (islower(c) ? c + ('A' - 'a') : c)

//
//	Manejo de cadenas de caracteres
//
extern char* strtoupper(char* s);
extern char* strchr	(const char* str, char chr);
char* strcpy(char* destino, const char* origen);
#pragma aux strcpy = \
		"push	di" \
		"b: lodsb" \
		"stosb" \
		"test 	al, al" \
		"jnz 	b" \
		"pop 	ax" \
		parm 	[di] [si] \
		modify 	[ax di si] \
		value 	[ax]
		
void* memcpy(void* dst, const void* src, WORD cnt);
#pragma aux memcpy = \
		"mov	ax, di" \
		"rep	movsb" \
		parm	[DI] [SI] [CX] \
		modify	[DI SI CX AX] \
		value	[AX]
		
		
//	Datos que se asume que tiene el BPB del disco.
#define BPB_LARGO_SECT		512
#define BPB_SECT_P_CLUST	1
#define BPB_SECT_RESERV		1
#define BPB_N_FATS			2
#define BPB_ENT_RAIZ		224
#define BPB_N_SECT			2880
#define BPB_MEDIA			0xF0
#define BPB_SECT_P_FAT		9
#define BPB_SECT_P_CIL		18
#define BPB_N_CABEZALES		2
#define BPB_EXTENDIDO		0x29
		
// Cuando se produce un error del cual no se puede rucuperar el sistema, se llama a esta funcion
extern void fatal(const char* msj);

// Entra en un bucle infinito
extern void infinito(void);

#endif //ifndef GENERAL_H
