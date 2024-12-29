
//
//	DABRIR
//	Inicializa un fichero para el directorio correspondiente a la ruta dada.
//	La ruta puede oorresponder a un archivo, en ese caso pasar TRUE en el parametro 'archivo' para
//	que la funcion termine recorrer la ruta cuando encuentre al subdirectorio que contiene al
//	arvchivo.
//	El fichero apuntado por fp debería estar vacio, ya que su contenido se modificara y en caso de
//  error, la funcion devuelve NULL y el contenido original de fp queda destruido.
//
//	ruta		Cadena de caracteres correspondiente a la ruta del directorio
//	fp			Puntero a estructura FICH que contendra la informacion del directorio abierto.
//	archivo		Indica si la ruta corresponde a un archivo o a un subdirectorio
//
//	DEVUELVE: Codigo de error
//
CODERR dabrir(const char* ruta, FICH* fp, BOOL archivo) {
	
	char nombre83[11];
	BOOL hayado;
	DIRENT *entp;
	CODERR e = E_EXITO;
	
	// Inicializa fp con el directorio raiz.
	fp->flags = F_DIR | F_RAIZ;
	drebo(fp);
	
	
	// Saltea el '.' inicial indicativo del directorio raiz.
	ruta++;
	
	// Recorre la ruta buscando en cada directorio al siguiente hasta terminar
	while(*ruta != '\0') {
			
		// Saltea el separador '/'
		ruta++;
		
		// Esta condicion se da cuando la ruta corresponde a la del directorio raiz "./"
		if(*ruta == '\0')
			break;
		
		// Obtiene el nombre del directorio que se debe buscar (en el formato en que se guardan en 
		// el sistema de arhivos) y avanza la posicion del puntero 'ruta'.
		ruta = ruta_a_nombre83(nombre83, ruta);
		
		// Si la ruta termina con un archivo (no un directorio) y estamos en esa posicion de la
		// ruta no hay que avanzar mas. Ejemplo: "./DIR/SUBDIR/ARCHIVO.TXT", hay que devolver
		// el FICH correspondiente a "SUBDIR"
		if(archivo && *ruta == '\0')
			break;
		
		// Busca el directorio de nombre 'nombre83' dentro directorio al que apunta el FICH
		hayado = FALSE;
		while(!(e = dleer(fp, &entp))) {
				
			if(entp->nombre[0] == '\0')
				return E_NOEXISTE; // Se llego al final del subdirectorio
			
			// Ignora las entradas "." y ".." dentro de los subdirectorios
			// Tambien ignora los archivos de etiqueta de unidad
			if(entp->nombre[0]!='.' && !(entp->atr & ATR_ETIQUETA)) {
				// Compara el nombre del archivo buscado con el de la entrada actual.
				if(!strncmp(entp->nombre, nombre83, 11)) {
					if(entp->atr & ATR_SUBDIR)
						hayado = TRUE;
					else
						e = E_NOEXISTE;
					
					break;
				}
			}
			
			// Avanza la posicion del FICH a la siguiente entrada.
			if(!davan(fp)) 
				return E_NOEXISTE;
		}
		
		// La busqueda dentro del directorio termino, hay que ver si fue porque se hayo el
		// subdirectorio buscado o porque no se encontro (o porque hubo un error).
		if(hayado) {
			// Hayamos el subdirectorio buscado, ahora fp contendra la informacion del mismo.
			
			// Carga primero la informacion de la entrada de directorio
			if(fp->flags & F_RAIZ)
				fp->entsect = SECINI_RAIZ + fp->clustn;
			else
				fp->entsect = CLUSTSECT(fp->clust);
			
			fp->entdesp = fp->pos % BYTES_CLUST;
			
			fp->flags = F_DIR;
			memcpy(&(fp->dirent), entp, BYTES_DIRENT);
			
			// fp ahora contiene la informacion del subdirectorio encontrado, hay que rebovinarlo
			// para buscar dentro de el en la proxima iteracion.
			drebo(fp);
			
		}
		else {
			return e;
		}
		
		// Llegados aqui, si el subdirectorio se encontro y no hubo errores, fp ahora contiene
		// un nuevo directorio de la ruta, el bucle comenzara de nuevo, buscando en el mismo
		// al siguiente. Asi, todo el dia, hasta terminar.
		
	}
	
	return E_EXITO;
}

//
//	DAVAN
//	Avanza la posicion del directorio a la proxima entrada.
//	Establece la posicion actual del directorio en la proxima entrada. Si el directorio se encuentra
//	posicionado en la ultima entrada.
//	Tener en cuenta que si una entrada de directorio comienza con el caracter nulo '\0' se llego 
//	al final del directorio. DAVAN no hace dicha comprobacion, y seguira avanzando hasta el final
//	del cluster o hasta el final del directorio raiz si es el caso. 
//
CODERR davan(FICH* fp) {

	if(!(fp->flags & F_DIR))	// Tiene que ser un directorio
		return E_ARGINVAL;
	if(fp->pos % BYTES_DIRENT)	// La posicion tiene que coincidir con una entrada de directorio
		return E_ARGINVAL;
		
	// Verifica si avanzar la posicion implica ir al siguiente cluster del directorio
 	if(fp->pos % BYTES_CLUSTER == BYTES_CLUST - BYTES_DIRENT) {
		
		if(fp->flags & F_RAIZ) { // Directorio raiz, tiene un tamano fijo
			if(fp->clustn == BPB_ENT_RAIZ - 1)
				return E_FINDIR;
		}
		else { // Directorio normal, hay que obtener el proximo cluster de la FAT
			WORD proxclust = fatprox(fp->cluster);
			if(proxclust == 1 || proxclust >= CLUST_MALO) 
				return E_FINDIR;
			fp->clust = proxclust;
		}
		
		// Nuevo cluster
		fp->clustn++;
	}
	
	// Avanza la posicion
	fp->pos += BYTES_DIRENT;
	
}

//	Rebobina el directorio.
void drebo(FICH* fp) {
	
	if(fp->flags & F_RAIZ) {
		fp->pos = fp->clustn = 0;
	}
	else {
		fp->clustn = fp->pos = 0;
		fp->cluster = fp->dirent.cluster;
	}
	
}

//	Actualiza el puntero de entrada pasado como argumento para que apunte a la informacion de la
//	entrada correspondiente a la posicion actual del fichero dado. Tener en cuenta que dicho
//	puntero apunta a inforamcion contenida en un bufer interno que puede ser modificado por
//	cualqueir otra operacion de lectura/escrtura. Usarlo poco tiempo, y si se quiere preservar
//	la informacion, copiarla a otro lugar.
CODERR dleer(FICH* fp, DIRENT** entpp) {
	WORD sect, desp;
	SECBUF* bp;
	
	// Tiene que ser un directorio
	if(!(fp->flags & F_DIR))
		return E_ARGINVAL;
	
	if(fp->flags & F_RAIZ)
		sect = SECINI_RAIZ + fp->clustn;
	else
		sect = CLUSTSECT(fp->clust);
	desp = fp->pos % BYTES_CLUST;
	
	if(!(bp = sec_obtener(sect)))
		return E_ERRODISCO; // El unico motivo por el que sec_obtener() puede fallar.
	
	*entpp = (DIRENT*) &(bp->buf[desp]);
	return E_EXITO;
	
}

//	Modifica la entrada correspondiente a la posicion actual del directorio.
CODERR descr(FICH* fp, DIRENT* entp) {
	WORD sect, desp;
	SECBUF* bp;
	CODERR e;
	
	// Tiene que ser un directorio
	if(!(fp->flags & F_DIR))
		return E_ARGINVAL;
	
	if(fp->flags & F_RAIZ)
		sect = SECINI_RAIZ + fp->clustn;
	else
		sect = CLUSTSECT(fp->clust);
	desp = fp->pos % BYTES_CLUST;
	
	if(!(bp = sec_obtener(sect)))
		return E_ERRODISCO; // El unico motivo por el que sec_obtener() puede fallar.
	
	memcpy(&(bp->buf[desp]), entp, BYTES_DIRENT); 
	
	bp->flags |= SEC_MODIF;	// Marca al bufer como modificado.
	
	// No se por que, pero guarda todos los buferes.
	return sec_guardtodos();
	
} 

//	Posiciona al FICH en la proxima entrada libre a partir de la posicion actual. Si llega al final
// y no encuentra una, expande al directorio (a menos que se trate del directorio raiz).
CODERR dnueva(FICH* fp) {
	
	DIRENT* entp;
	CODERR e;
	
	do {
		// Obtiene la entrada correspondiente a la posicion actual.
		if(!(e = dleer(fp, &entp)))
			return e;
		
		// Verific si se trata de una entrada disponible para ser sobreescrita.
		if(entp->nombre[0] == '\0' || entp->nombre[0] == BORRADO) 
			return E_EXITO; // Entrada libre encontrada.
		
		// Llegando aqui, la entrada actual no esta libre. Hay que avanzar a la siguiente posicion.
	} while(!(e = davan(fp)));
		
	// Si no se hayo ninguna entrada libre. Intenta expandir el directorio.
	
	if(fp->flags & F_RAIZ)
		return E_FINDIR; 	// EL directorio raiz no se puede expandir.
	
	if(e != E_FINDIR) // Si el problema no fue que se hayo el final del directorio se debe salir
		return e;     // el codigo a continuacion asume que el directorio se puede expandir
					  // porque davan() lo dejo posicionado al final del ultimo cluster.
	
	// Obtiene un cluster libre dentro del sistema de archivos.
	WORD clustlibre = fatlibre();	
	if(clustlibre == 1) return E_DSCLLENO;
	
	// Agrega el cluster a la cadena de clusteres.
	fatclust(fp->clust, clustlibre);
	fatclust(clustlibre, CLUST_FINAL);
	
	// Nada deberia fallar en esta parte. Como ya se expandio el directorio, se avanza a la
	// siguiente posicion.
	if(!(e = davan())) return e;
	
	return E_EXITO;
}
