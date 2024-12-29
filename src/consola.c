#include "misc.h"

// Macro para hacer avanzar el cursor.
#define AVANZAR if(!inccur()) nuevalinea()
				
#define DEF_ATR  7	// Atributo utilizado por defecto al escribir en la pantalla
#define DEF_MODO 2	// Modo de video utilizado por defecto.

// Ultima fila y columna de la pantalla. Coincidentes con el modo DEF_MODO.
#define ULTFILA	24
#define ULTCOLU	79

// Codigo de escaneo del teclado devuelto por HIBYTE(esperartecla())
#define TECLA_ENTER		0x1C
#define TECLA_BACKSPACE 0x0E

///////////////////////////////////////////////////////////////////////////////////////////////////
// Interrupciones del BIOS
///////////////////////////////////////////////////////////////////////////////////////////////////

//
// Establece el modo de video. 
//
void modovideo(BYTE);
#pragma aux modovideo = \
		"xor	ah, ah" \
		"int	10h" \
		parm	[al] \
		modify	[ax];

//
// Establece la posición del cursor.
//
void fijarcursor(BYTE x, BYTE y);
#pragma aux fijarcursor = \
		"mov	bh, 0" \
		"mov	ah, 2" \
		"int	10h" \
		parm	[dl] [dh] \
		modify	[ah bh]
		
//
// Devuelve la posicion del cursor. Columna en LOBYTE y fila en HIBYTE
//
WORD obtcursor(void);
#pragma aux obtcursor = \
		"mov	bh, 0" \
		"mov	ah, 3" \
		"int	10h" \
		value	[dx] \
		modify	[ah cx bh];
	
//
// Escribe un caracter y su atributo en la posicion del cursor
//
void escrcaratr(char car, BYTE atr );
#pragma aux escrcaratr = \
		"mov	bh, 0" \
		"mov	cx, 1" \
		"mov	ah,	9" \
		"int	10h" \
		parm [al] [bl] \
		modify [cx ah bh];
	
//
// Desplaza n filas hacia arriba dentro del recuadro (x,y,ancho,alto) aplicandole a las nuevas
// filas el atributo atr.
//
void desparriba(BYTE n, BYTE atr, BYTE x, BYTE y, BYTE ancho, BYTE alto);
#pragma aux desparriba = \
		"mov	ah, 6" \
		"int	10h" \
		parm	[al] [bh] [cl] [ch] [dl] [dh] \
		modify  [ah];

//
// Espera a que el usuario presione una tecla.
// Si LOBYTE == 0: HIBYTE = codigo ASCII extendido.
// Si LOBYTE != 0: LOBYTE = codigo ASCII. HIBYTE = codigo de escaneo.
//
WORD esperartecla(void);
#pragma aux esperartecla = \
		"xor	ah, ah" \
		"int	16h" \
		value [ax];
				
///////////////////////////////////////////////////////////////////////////////////////////////////
// Funciones para hacer avanzar o retroceder la posicion del cursor (no agregan nuevas lineas)
///////////////////////////////////////////////////////////////////////////////////////////////////				

//
//	DECCUR
//	Si es posible, mueve el cursor una posición hacia atrás.
//
//	DEVUELVE: TRUE si se pudo mover el cursor, FALSE si el cursor estaba en (0,0)
//
BOOL deccur() {
	WORDHL c;
	c.word = obtcursor();
	
	if(c.word == 0) return FALSE;
	
	if(c.byte.lo == 0) {
		c.byte.hi--;
		c.byte.lo = ULTCOLU;
	}
	else
		c.byte.lo--;
	
	fijarcursor(c.byte.lo, c.byte.hi);
	return TRUE;
}

//
//	INCCUR
//	Si es posible, mueve el cursor una posición hacia adelante.
//
//	DEVUELVE: TRUE si se pudo mover el cursor, FALSE si el cursor estaba al final de la pantalla.
//
BOOL inccur() {
	WORDHL c;
	c.word = obtcursor();
	
	if(c.word == MKWORD(ULTFILA, ULTCOLU)) return FALSE;
	
	if(c.byte.lo == ULTCOLU) {
		c.byte.hi++;
		c.byte.lo = 0;
	}
	else
		c.byte.lo++;
	
	fijarcursor(c.byte.lo,c.byte.hi);
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Funciones declaradas en CONSOLA.H
///////////////////////////////////////////////////////////////////////////////////////////////////

// 
//	INI_CONSOLA
//	Rutina de inicializacion de la consola.
//
void ini_consola(void) 
{
	modovideo(DEF_MODO);// Establece el modo de video
	fijarcursor(0,0);	// Establece la posicion del cursor
}

//
//	NUEVALINEA
//  Avanza a la siguiente linea, si el cursor se encuentra en la ultima linea, desplaza la 
//  pantalla hacia arriba.
//
void nuevalinea(void) {
	WORDHL c;
	c.word = obtcursor();
	if(c.byte.hi == ULTFILA) {
		desparriba(1,0,0,0, ULTCOLU, ULTFILA);
		fijarcursor(0, ULTFILA);
	}
	else {
		fijarcursor(0, c.byte.hi+1);
	}
	
}

// 
// 	CLEERX
// 	Lee una linea de texto desde el teclado, restringida a un determinado juego de caracteres.
//
//	buf		Puntero al bufer donde se almacenara la cadena.
//	n		Tamano del bufer (tener en cuenta el '\0' final)
//	x		Puntero a la cadena que contiene el juego de caracteres permitdo. Si es un puntero nulo
//			se permiten todos los caracteres ASCII imprimibles.
//
// 	DEVUELVE: El puntero al bufer donde se almaceno la cadena.
//
char* cleerx(char* buf, INT16 n, const char* x) {
	
	// Obtiene la posicion actual del cursor en cursor y tiene un contador c.
	// Mediante un bucle lee las teclas que presiona el usuario.
	// 	Ignora todas las teclas excepto: enter, backspace, caracteres ASCII imprimibles.
	//	ENTER 	  - El usuario finalizo de ingresar la cadena. FIN.
	//	BACKSPACE - Se borra el ultimo caracter ingresado a menos que estemos en el comienzo.
	//	ASCII	  - Si esta en x, se agrega el caracter al bufer, a menos que ya se hayan leido n-1
	//				caracteres.
	
	
	WORDHL cursor;
	register INT16 c = 0;
	register WORDHL tecla;
	
	// Obtiene la posicion del cursor donde se empieza a escribir.
	// Se usa para detener el borrado de caracteres cuando el usuario sigue apretando BACKSPACE
	cursor.word = obtcursor();
	
	while(TRUE) {
		
		tecla.word = esperartecla();	// Espera a que el usuario presione una tecla.
		if(!tecla.byte.lo) continue; 	// Teclas raras son ignoradas
		
		switch (tecla.byte.hi) 
		{
			case TECLA_ENTER:
				buf[c] = '\0';
				nuevalinea();
				return buf;
				
			case TECLA_BACKSPACE:
				if(cursor.word == obtcursor()) continue; // Si esta al comienzo del texto o al co_
				if(!deccur()) continue;			   		 // mienzo de la pantalla ignora la tecla
				c--;
				escrcaratr(0, 0); // Borra el caracter
				break;
				
			default:
				// Verifica que el caracter sea imprimible, que haya lugar en el bufer, y que el
				// caracter se encuentre dentro del juego de caracteres aceptados.
				if(!isprint(tecla.byte.lo)) continue;
				if(c == n) continue;
				if(x!=NULL){if(strchr(x, tecla.byte.lo)==NULL) continue;}
				
				// Agrega el caracter al bufer y lo muestra en pantalla.
				buf[c] = tecla.byte.lo;
				escrcaratr(tecla.byte.lo, DEF_ATR);
				
				// Adelanta el cursor de la pantalla, si es posible.
				if(!inccur()) {
					if(cursor.byte.hi > 0) {
						cursor.byte.hi--;
						nuevalinea();
						c++;
					}
				}
				else
					c++;
				
				break;	
		}
	}
	
}

//
//	CESCR
//	Escribe una cadena en pantalla. Trata a '\n' como caracter de nueva linea.
//
//	msj		Puntero a la cadena terminada en 0 que se pretende escribir.
//
void cescr(const char* msj) {
	register const char* p;
	for(p = msj; *p != '\0'; p++) {
		if(*p == '\n')
			nuevalinea();
		else {
			escrcaratr(*p, DEF_ATR);
			AVANZAR;
		}
	}
}

//
//	CLEERCX
//	Espera hasta que el usuario presione una tecla coincidente con un caracter ASCII imprimible 
//  contenido en el juego de caracteres dado. Pasar x = NULL si se desesa que se acepten todos los
//	caracteres ASCII imprimibles.
//
//	x		Juego de caracteres aceptados. Si es NULL se aceptan todos los caracteres ASCII.
//
//	DEVUELVE: El valor ascii del caracter leido desde el teclado.
//
char cleercx(const char* x) {
	register WORDHL tecla;
	
	while (TRUE) 
	{
		// Espera a que el usuario presione una tecla
		tecla.word = esperartecla();
		
		// Verifica que el caracter sea imprimible y que se encuentre dentro del juego de
		// caracteres dado.
		if(!isprint(tecla.byte.lo)) continue;
		if(x != NULL) {if(strchr(x, (char)tecla.byte.lo)==NULL) continue; }
		
		// Muestra el caracter en pantalla
		escrcaratr(tecla.byte.lo, DEF_ATR);
		AVANZAR;
		
		return tecla.byte.lo;
	}
}

//
//	CESCRC
//	Muestra un caracter en pantalla. El caracter '\n' avanza a la siguiente linea.
//
//	c		Caracter ASCII que se pretende mostrar en la pantalla.
//
void cescrc(char c) {
	
	if(c == '\n') {
		nuevalinea();
	}
	else {
		if(isprint(c)) escrcaratr(c, DEF_ATR);
		AVANZAR;
	}
}

//
//	CLIMPIAR
//	Limpia la pantalla.
//
void climpiar(void) {
	desparriba(ULTFILA+1,0,0,0, ULTCOLU, ULTFILA);
	fijarcursor(0,0);
}

//
//	CLEERI
//	Lee un numero entero decimal desde el teclado.
//
//	DEVUELVE: El numero entero escrito en el teclado. Tener en cuenta que el numero 0 puede
//	significar que hubo un error.
//
INT cleeri(void) {
	
	char b[BUFER_ITOA];
	return atoi(cleerx(b, BUFER_ITOA, "+-0123456789"), 10);
}

//
//	CESCRI
//	Muesra un numero entero en pantalla expresado en la base dada.
//
//	n		Numero entero que se desea escribir
//	base	Base en la que se desea mostrar el numero
//
void cescri(INT n, INT base) {
	
	char b[BUFER_ITOA];
	cescr(itoa(n, b, base));
}

