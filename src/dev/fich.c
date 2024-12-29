
struct struct_fich {
	WORD 	flags;		// Indicadores
	
	// Codigo de error de la ultima operacion sobre el fichero.
	CODERR  coderr;
	
	WORD 	pos;		// Posicion absoluta dentro del archivo
	WORD 	clustn;		// Numero de cluster actual realitvo al archivo
	
	WORD 	cluster;		// Cluster actual en la fat.
	
	DIRENT 	dirent;		// Entrada de directorio del archivo
	
	// Esto es neceario por si se necesita acutalizar la informacion dentro de la entrada
	WORD	entsect;	// Sector donde se encuentra la entrada
	WORD	entdesp;	// Desplazamiento dentro del sector
}

#define F_LIBRE		0
#define F_RAIZ		0x08	// Es el directorio raiz
#define F_DIR		0x10	// Es un directorio (incluye al raiz)

WORD fleer(FICH* f, WORD n, BYTE* buf) {
	WORD tot, este;
	WORD desp;
	WORD clust;
	BOOL ultimo = FALSE;
	SECBUF* bp;
	
	tot = este = 0;
	do {

		// Obtiene el sector
		bp = sec_obtener(CLUSTSECT(f->clust));
		if(!bp)
			return tot;
		
		bp->flags &= ~(SEC_FAT | SEC_RAIZ);
		bp->flags |= SEC_DATOS;
		
		
		
		// Obtiene el desplazamiento dentro del sector y la cantidad max de bytes leibles en el se
		desp = f->pos % PBP_BYTES_P_SECT;
		este = BPB_BYTES_P_SECT-desp;
		
		// Si 'este' supera o iguala lo que queda en el bufer, hay que leer menos.
		if(tot+este>=n) {
			este = n-tot;
			ultimo = TRUE;
		}
		
		// Si 'este' supera el tamano del archivo, hay que leer menos.
		if(f->pos+este >= f->dirent.tamano) {
			este = (WORD) f->dirent.tamano-f->pos;
			utlimo = TRUE;
		}
		
		// Copia la informacion
		memcpy(buf, &(bp->buf[desp]), este);
		
		if(ultimo)
			return tot;
		
		// Si llegamos aca hay que seguir leyendo en el proximo cluster
		
		// Obtiene el proximo cluster
		clust = fatclust(f->clust, LEER_CLUSTER);
		if(/* no hay mas clusters para leer*/)
			return tot;
		
		bp->flags |= SEC_LISTO;
		
		// Actualiza informacion
		f->clust = clust;
		f->clustn++;		// Numero de cluster
		f->pos+=este;		// Actualiza la posicion
		
		// Contador de bytes totales leidos
		tot+=este;
		buf = &(buf[este]);
		
	} while(tot < n);
	
	return tot;
}


CODERR fabrir(const char* ruta, WORD modo, FICH* fp) {
	CODERR e;
	
	if(!(fp = fichlibre()))
		return E_SINFICH;
	
	e = fbuscar(ruta, fp);
	if(!e)
		return e;
	
	if(!(modo & MODO_DIR) && fp->flags & F_DIR)
		return E_NEXISTE;
	
	if(!(fp->flags & F_RAIZ))
		bloq(fp->dirent.cluster);
	
	fp->flags |= F_ABIERTO;
	
	return fp;
}

void fcerrar(FICH* fp) {
	
	if(!(fp->flags & F_RAIZ))
		desbloq(fp->dirent.cluster);
	
	/* FALTARIA GUARDAR LOS BUFERES ABIERTOS MODIFICADOS */
	
	fp->flags = F_LIBRE;
}

// Busca el archivo espeficicado por la ruta e inicializa el FICH con la infomracion para dicho
// archivo.
CODERR fbuscar(const char* ruta, FICH* fp) {
	
	CODERR e = E_EXITO;
	BOOL hayado = FALSE;
	char nombre83[11];
	DIRENT* entp;

	
	// Completa con la informacion para el directorio raiz.
	fp->flags = F_DIR | F_RAIZ;
	fp->pos = fp->clustn = 0;
	
		
	while(*ruta != '\0') {
		
		ruta++; // Salta el separador
		
		// Si en el medio de la ruta, hay un archivo que no es un directorio, la ruta esta mal.
		if( !(fp->flags & F_DIR) )
			return E_RUTAMALA;
		
		// Si luego de saltar el separador, estamos en el final, la ruta correspondia al directorio
		// raiz.
		if(*ruta == '\0')
			break;
		
		// Convierte el nombre del archivo al formato 8.3 en el que se guardan en la entrada
		// para poder comparar. Ahora ruta apunta al proximo '\' o al '\0' final si era el ultimo.
		ruta = nombre_a_83(nombre83, ruta);
		
		hayado = FALSE;
		
		// Para cada entrada dentro del directorio
		while(!(e = dproxent(fp, &entp))) {
			if (entp == NULL)
				break;
			// Tener en cuenta que entp apunta a la entrada dentro del SECBUF. Por lo tanto
			// cualquier otra operacion de lectura/escritura de sector puede destruir el puntero
			
			// Saltea las entradas '.' y '..' desntro del directorio
			if(entp->nombre[0] == '.')
				continue;
			
			// Encontramos el archivo?
			if(!strncmp(entp->nombre, nombre83, 11) && !(entp->atr & ATR_ETIQUETA)) {
				hayado = TRUE;
				break;
			}
		}
		
		
		
		if(hayado) {
			
			// Completa el FICH con la informacion del nuevo archivo encontrado.
			
			if(fp->flags & F_RAIZ) 
				fp->entsect = fp->clustn;
			else
				fp->entsect = CLUSTSECT(fp->clust);
			
			fp->entdesp = fp->pos % BPB_BYTES_P_SECTOR;
			
			// Copia la info de la entrada
			memcpy(&(fp->dirent), entp, BYTES_DIRENT);
			
			fp->clust = fp->dirent.cluster;
			fp->clustn = cp->pos = 0;
			if(fp->dirent.atr & ATR_SUBDIR) 
				fp->flags = F_DIR;
		}
		else {
			if(e && e != E_FINFICH) // Codigo de error devuelto por la funcion dproxent()
				return e;
			else 
				return E_NOEXISTE; 
		}
	}
	
	return E_EXITO;
	
}
