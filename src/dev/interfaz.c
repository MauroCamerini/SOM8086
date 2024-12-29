#include "som8086.h"

//
//	Lista de comandos
//
extern void cmd_ayuda	(INT argc, char** argv);

#define MAX_ARGC 	16
#define MAX_LINEA 	128

struct struct_args {
	INT		argc;
	char* 	argv[MAX_ARGC];
};

typedef struct struct_args ARGS;

struct struct_cmdinter {
	
	const char* nombre;
	COMANDO 	comando;
};

typedef struct struct_cominter CMDINTER;

#define LARGO_BUFER 16
CMDINTER *internos;
//
//	INI_INTERFAZ
//	Inicializa el modulo
//
CODERR ini_interfaz(void) {
	
	internos = reslocal(sizeof(CMDINTER) * LARGO_BUFER);
	if(!internos) return E_RESMEM;
	
	internos[0].nombre 	= "AYUDA";
	internos[0].comando = &cmd_ayuda;
	
	internos[1].nombre 	= "SALIR";
	internos[1].nombre 	= &cmd_salir;
	
	internos[2].nombre 	= '\0';
	internos[2].comando = NULL;
}

//
//	INTERFAZ
//	Funcion principal de interprete de linea de comandos
//	El nucleo, una vez que inicializ todos los modulos, pasa el control a la interfaz de usuario
//	por intermedio de esta funcion.
//
void interfaz(void) {
	
	ARGS args;
	char linea[MAX_LINEA];
	
	while(TRUE) {
		
		cleer(linea, MAX_LINEA);	// Lee una linea desde el teclado
		strtoupper(linea);			// La pasa a mayusculas
		
		// Obtiene los argumentos;
		if(!obtener_args(&args, linea)) {
			cescr("Los comandos pueden tener como maximo ");
			cescri(MAX_ARGC, 10);
			cescr(" arguemntos.\n");
			continue;
		}
		
		// Obtiene el comando
		COMANDO cmd = obtener_comando(&args);
		if(!cmd) {
			cescr("El comando no existe.\n");
			continue;
		}
		
		// Ejecuta el comando.
		cmd(args.argc, args.argv);
	}
}

//
//	OBTENER_ARGS
//	Analiza una linea leida desde el teclado y completa la estructura ARGS con los argumentos
//	contenidos en la misma. La cadena linea es modificada porque se agrega un '\0' al final de cada
//	argumento.
//
//	ap		Puntero a la estructura ARGS que contendra la informacion de los comandos.
//	linea	Cadena de caracteres obtenida desde el teclado.
BOOL obtener_args(ARGS* ap, char* linea) {
	
	ap->argc = 0;
	
	while(ap->argc < MAX_ARGC) 
	{
		// Saltea los espacios en blanco.
		if(isspace(*linea)) linea++;
		
		// Verifica que todavia haya argumentos.
		if(*linea == '\0') break;
		
		// Agrega el argumento
		ap->argv[ap->argc] = linea;
		ap->argc++;
		
		// Va hasta el final del argumento
		while(!isspace(*linea) && *linea != '\0') linea++;
		
		if(*linea == '\0') {	// No hay mas argumentos
			break;
		}
		else 					// Todavia hay. Agrega el caracter nulo y avanza.
		{
			*linea= '\0';
			linea++;
		}
	}
	
	if(ap->argc >= MAX_ARGC)
		return FALSE;
	else
		return TRUE;
	
}

COMANDO obtener_comando(ARGS* args) {
	
	CMDINTER *cmdp;
	
	if(args->argc < 1)
		return NULL;
	
	for(cmdp = internos; cmdp->nombre[0]; cmdp++) {
		if(!strcmp(cmdp->nombre, args->argv[0]))
			return cmdp->comando;
	}
	
	return NULL;
}
