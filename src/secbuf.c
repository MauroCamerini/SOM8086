#include "som8086.h"

//	Primer bufer de la lista
//	Los buferes estan listados en base a sus campos prox y prev que apuntan al bufer siguiente y
//	anterior respectivamente. El campo prox del ultimo bufer apunta al primero, por lo tanto la
//	lista es circular. Esto se hizo asi para simplificar la tarea de mover los buferes de posicion
//	dentro de la lista. 
//	Cada vez que se solicita un sector mediante sec_obtener(), se pone a este primero en la lista. 
//	De esta manera, el orden de la lista coincide con la frecuencia de uso de los buferes. Estan
//	primeros los mas utilizados. Esto nos permite, en el caso de tener que reutilizar un bufer,
//	optar por el ultimo que es el que no se esta usando.
BUFSECT* primero = NULL;

//
//	INI_SECBUF
//	Inicializa el modulo SECBUF.
//	Reserva memoria para almacenar los buferes. Crea la lista enlazada. Marca todos los buferes
//	como libres.
//
//	DEVUELVE: E_EXITO.
//
//	ERROR:
//	E_RESMEM
//
CODERR ini_secbuf(void) {
	
	WORD	c;
	SECBUF* bp, *prev;
	
	// Reserva memoria para los buferes
	bp = primero = reslocal(sizeof(SECBUF)*N_BUFERES);
	
	if(!primero)
		return E_RESMEM;
	
	// Enlaza los buferes.
	prev = NULL;
	for(c=0; c<SEC_N_SECBUF; c++) {
		bp->flags = SEC_VACIO;
		
		if(c==SEC_N_SECBUF-1) { // Ultimo sector
			bp->prox = primero;
			primero->prev = b;
		}
		else {
			bp->prev = prev;
			bp->prox = bp+1;
		}
		
		prev = bp;
		bp++;
	}
	
	return E_EXITO;
}

//
//	PONERPRIMERO
//	Pone al bufer pasado como argumento primero en la lista de buferes.
//
void ponerprimero(SECBUF* bp) {
	
	
	if(bp==primero) 
		return; // Ya es el primero, no hace nada.
	
	// Extrae el sector de donde esta
	bp->prox->prev = bp->prev;
	bp->preb->prox = bp->prox;
	
	// Lo inserta en el primer lugar
	primero->prev->prox = bp;
	primero->prox->prev = bp;
	primero = bp;
	
}

//
//	SEC_OBTENER
//	Devuelve el bufer correspondiente al sector solicitado.
//	Verifica si el sector ya esta cargado en un bufer. Si no, carga el sector en ultimo bufer 
//	marcado como listo, o en su defecto, en el ultimo bufer de la lista.
//	En el caso de sobreescribir un bufer utilizado, guarda el contenido el bufer si estaba marcado
//	como modificado.
//	El bufer devuelto es puesto primero en la lista, asi los buferes estan ordenados desde los
//	recientemente soloicitados, hasta los solicitados anteriormente.
//
//	sect		Numero de sector que se desea obtener
//
//	DEVUELVE: Un puntero al bufer que contiene al sector solicitado.
//
//	ERROR:
//	NULL		Error de lectura/escritura del disco.
//
SECBUF* sec_obtener(WORD sect) {

	SECBUF* bp = primero;
	SECBUF* listo = NULL;
	
	// Busca entre los buferes
	do {
		// Encontro al sector solicitado en un bufer
		if(bp->flags && bp->sect == sect) {
			ponerprimero(bp);		 // Lo pone primero en la lista.
			bp->flags &= ~SEC_LISTO; // Elimina la marca de listo.
			return bp;
		}
		// Encontro un sector marcado como listo
		else if(bp->flags & SEC_LISTO)) {
			listo = bp;
		}
		
		bp = bp->prox; 		// Siguiente bufer
	} while(bp != primero); // Hasta que vuelve al primero
	
	// Si llegamos aca el sector no esta cargado en ningun bufer.
	
	// Toma el ultimo bufer listo
	if(listo) {
		bp->flags &= ~SEC_LISTO;
		bp = listo;
	}
	// Si no hay, toma el ultimo bufer de la lista (el menos utilizado)
	else {
		bp = primero->prev;
	}
	
	// Antes de sobreescribir el bufer, guarda el contenido si fue modificado.
	if(bp->flags & SEC_MODIF) {
		if(!sec_guardar(bp))
			return NULL;
	}
	
	// Carga el sector en el bufer
	if(!opdisco(DSC_LEER, sect, bp->buf)) 
		return NULL;
		
	bp->flags = SEC_OCUPADO | SEC_DATOS; // Por defecto lo marca como sector de datos.
	bp->sect = sect;
	
	ponerprimero(bp);	// Lo pone primero en la lista
	
	return bp;
		
}

//
//	SEC_GUARDAR
//	Escribe en el disco el contenido del bufer. Si se trata de un sector maracado como FAT, lo
//	escribe en las dos copias de la FAT
//
//	bp			Puntero al bufer de sector
//
CODERR sec_guardar(SECBUF* bp) {
	
	// Guarda el bufer en el sector correspondiente en el disco
	if(!opdisco(DSC_ESCR, bp->sect, bp->buf))
		return E_ERRDISCO;
	
	// El bufer ya no figura como moficado.
	bp->flags&= ~SEC_MODIF;
	
	// Si el bufer contiene un sector de la fat, se debe guardar la otra copia tambien
	if(bp->flags & SEC_FAT) {
		if(!opdisco(DSC_ESCR, bp->sect+BPB_SECT_P_FAT, bp->buf))
			return E_FATCOPIA;
	}
	
	
	return E_EXITO;
}

//
//	SEC_GUARDTODOS
//	Escribe en el disco el contenido de todos los buferes marcados como modificados.
//
//	bp			Puntero al bufer de sector
//
//	DEVUELVE: El puntero bp.
//
//	ERROR:
//	NULL		Error de lectura/escritura del disco.
//
CODERR sec_guardtodos(void) {
	SECBUF* bp = primero;
	
	do {
		if(bp->flags & SEC_MODIF){	// Escribe el bufer en el disco, solo si fue modificado
			if(!sec_guardar(bp))
				return E_ERRDISCO;
			bp->flags &= ~SEC_MODIF;
		}
		bp = bp->prox;	
	} while(bp!=primero);
		
	return E_EXITO;
}

//
//	SEC_VACIARTODOS
//	Desocupa todos los buferes.
//	Se utiliza cuando se cambia el disco o es necesario reiniciar la unidad de disco por algun
//	error de lectura o escritura.
//
void sec_vaciartodos(void) {
	
	SECBUF* bp = primero;
	
	do {
		bp->flags = SEC_VACIO;
		bp = bp->prox;	
	} while(bp!=primero);
}
