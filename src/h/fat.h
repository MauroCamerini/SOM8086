#ifndef FAT_H
#define FAT_H


//
// Como SOM8086 esta programado estrictmente para un solo tipo de unidad. Se pueden establecer
// como constantes los siguientes datos.
//
#define SECTORARRANQUE	0 
// Secctor del disco donde comienza ...
#define SECINI_FAT1  	1	// ... la primera FAT
#define SECINI_FAT2  	2   // ... la segunda FAT
#define SECINI_RAIZ  	19	// ... el directorio raiz
#define SECINI_DATOS 	33 	// ... el area de datos
// Tamano en sectores de ...
#define LARGO_FAT		9	// ... cada una de las FAT
#define LARGO_RAIZ		14	// ... el directorio raiz.
#define LARGO_DATOS		2847// ... el area de datos

//
// Entrada de directorio
//

#define ENT_NOMBRE_SZ 8
#define ENT_EXT_SZ 	  3

struct struct_entdir{
	char 	nombre[ENT_NOMBRE_SZ];	// Nombre del archivo
	char 	ext[ENT_EXT_SZ];		// Extension
	BYTE 	atr;		// Atributos
	BYTE 	resevado[10];
	WORD 	tiempo;		
	WORD 	fecha;
	WORD 	cluster;
	DWORD 	tamano;
};


typedef struct struct_entdir DIRENT;
#define BYTES_DIRENT 	32

#define BORRADO 0xE5

#define BYTES_CLUST		BPB_BYTES_P_SECTOR

// Atributos para el campo DIRENT.atr
#define ATR_SOLOLECT	0x01
#define ATR_OCULTO		0x02
#define ATR_SISTEMA		0x04
#define ATR_ETIQUETA	0x08
#define ATR_SUBDIR		0x10
#define ATR_ARCHIVO		0x20

// Devuelve el sector del disco correspondiente al cluster dado
#define CLUSTSECT(clust) (clust-2+SECINI_DATOS)

// Valor que indica a la funcion fatclust() que debe devolver el valor del cluster
#define LEER_CLUSTER	1

// Maximo valor aceptado para un cluster de datos valido
#define CLUSTER_MAX LARGO_DATOS+2

// Este valor indica un cluster erroneo o incorrecto. Si el valor de un cluster es mayor a este
// numero, indica el final de la cadena de clusteres
#define CLUST_MALO	0xFF7


#define CLUST_LIBRE 0		// El cluster esta libre
#define CLUST_FINAL 0xFFF	// Final de la cadena de clusteres.

// Bios parameter block
struct struct_bpb {
	BYTE jmpnop[3];		// Instruccion de salto
	BYTE so[8];			// Nombre del sistema operativo
	WORD largo_sect;	// Tamano en bytes de cada sector del disco
	BYTE sect_p_clust;	// Cantidad de sectores que ocupa un cluster del sistema de archivos
	WORD sect_reserv;	// Sectores reservados
	BYTE n_fats;		// Numero de copias de la FAT
	WORD ent_raiz;		// Numero de entradas en el directorio raiz
	WORD n_sect;        // Cantidad total de sectores en el disco
	BYTE media;         // Indica el tipo de unidad
	WORD sect_p_fat;    // Cantidad de sectores que ocupa cada FAT
	WORD sect_p_cil:    // Numero de sectores por cada cilindro fisico del disco.
	WORD n_cabezales;   // Cantidad de cabezales
	DWORD sect_ocult;	// Sectores ocultos. Ninguno.
	DWORD n_sect32;		// Numero total de sectores en 32-bits, utilizado por unidades mas grandes.
	BYTE unidad;		// Numero de unidad
	BYTE reservado;
	BYTE extendido;		// Indica que esta disponible la siguiente informacion
	DWORD nserie;		// Numero de serie.
	BYTE etiqueta[11];  // Etiqueta del disco
	BYTE sf[8];			// Sistema de ficheros.
}

extern WORD 	fatclust	(WORD clust, WORD clust2);
extern WORD 	fatprox		(WORD clust);
extern CODERR 	fatborrar	(WORD clust);
extern WORD 	fatlibre	(void);
extern BOOL 	fat_bpbbueno(void);

extern const char* ruta_a_nombre83(char* nombre83, const char* ruta);

#endif
