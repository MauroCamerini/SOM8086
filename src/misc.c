#include "misc.h"

// Usado por la funcion itoa()
const char digit_alpha[] = "0123456789abcdefghijklmnopqrstuvwxyz";

//
//	STRTOUPPER
//	Pasa a mayusculas la cadena de caracteres dada.
//
char* strtoupper(char* s) {
	char* sp = s;
	while(*sp) {
		if(islower(*sp)) 
			*sp = toupper(*sp);
		sp++;
	}
	return s;
}

//
//	ITOA
//	Convierte en un numero ASCII en una cadena de caracteres
//
//	n		Numero entero que se desea convertir
//	p		Puntero a donde se almacenara la cadena.
//	base	Base en la que se desea expresar el numero.
//
//	DEVUELVE: El puntero p.
//
char* itoa(INT n, char* p, INT base) {
	char buf[BUFER_ITOA];	
	register char* bp;
	INT signo = 0;
	
	// Guarda el signo y torna a n positivo
	if((signo=n) < 0)
		n = -n; 
	
	bp = &(buf[BUFER_ITOA-1]);
	*bp = '\0';
	
	// Obtiene la cadena y la almacena en el bufer
	do {
		bp--;
		*bp = digit_alpha[n%base];
	} while((n/=base) != 0);
	
	// Aplica el signo
	if(signo<0) {
		bp--;
		*bp = '-';
	}
	
	// Copia la cadena del bufer al puntero dado.
	return strcpy(p, bp);
}

//
//	ATOI
//	Convierte una cadena ASCII compuesta caracteres alfanumericos en un numero entero respetando
//	la base dada.
//
//	p		Puntero a la cadena ASCII.
//	base	Base en la que se encuentra expresado el numero entero.
//
//	DEVUELVE: Un numero entero.
//
INT atoi(const char* p, INT base) {
	INT signo = 1, d, n = 0;
	
	// Ignora los espacios en blanco
	while(isspace(*p)) p++;
	
	// Verifica el signo
	if(*p == '-') {
		signo = -1;
		p++;
	}
	else if(*p == '+') {
		p++;
	}
		
	
	// Considera los digitos, descartando el resto de los caracteres si los hay.
	while(isalnum(*p)) {
		
		// Obtiene el nuevo digito
		d = isdigit(*p) ? (INT)(*p-'0') : (INT)(tolower(*p)-'a'+10);
		
		
		if(d>=base)break; // Verifica que el digito corresponda con la base dada
		n *= base;		  // Desplaza los digitos hacia la derecha para hacer lugar al nuevo.
		n += d;			  // Agrega el nuevo digito.
		p++;
	}

	// Aplica el signo y devuelve
	return n * signo;
}

//
//	STRCHR
//	Devuelve el puntero a la primera apricion del caracter c dentro de la cadena s. Si no lo
//	encuentra devuelve NULL. Se acepta c == '\0', por lo tanto se puede usar esta funcion para
//	encontrar el final de la cadena.
//
//	s		Puntero a la cadena donde se va a abuscar el caracter.
//	c		Caracter que se quiere encontrar.
//
//	DEVUELVE: El puntero al primer c dentro de s, o NULL.
//
char* strchr(const char* s, char c) {
	do {
		if(*s==c) return (char*)s;
		s++;
	} while(*s);
	return NULL;
}

