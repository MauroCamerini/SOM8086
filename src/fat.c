#include "som8086.h"

//
//	FATCLUST
//	Lee/modifica la entrada del cluster dado en la FAT.
//
//	clust		Cluster que se desea leer o modificar.
//	clust2		Determina si se desea leer un cluster, o el valor que se guardara en clust.
//
//	DEVUELVE: Valor del cluster leido/modificado.
//	1	En caso de error.
//
WORD fatclust(WORD clust, WORD clust2) {
	
	SECBUF* bp, *bp2 = NULL;
	WORD sect, desp;
	BYTE *lop, *hip;
	
	if(clust <= 1 || clust > CLUSTER_MAX)
		return 1;

	// TODO: verificar los valores de clust2
	
	// Obtiene el sector de la fat donde se encuenta el cluster
	// y el desplazamiento dentro del sector.
	sect = (clust-2) + (clust-2)/2;
	desp = sect%BPB_LARGO_SECT;
	sect /= BPB_LARGO_SECT;
	
	if(!(bp = reqsect(fatbuf, sect)))
		return 1;
	
	// Marca al bufer como FAT
	bp->flags &= ~SEC_DATOS;
	bp->flags |= SEC_FAT;
	
	// Punteros a los bytes menos y mas significativos del cluster.
	lop = &(bp->buf[desp]);
	hip = lop+1;
	
		
	// Lee o escribe la entrada de acuerdo a lo solicitado
	// Aqui entra lo complicado del sistem FAT12. Como utiliza numeros de cluster de 12bits, estos
	// se almacenan 8bits en un byte y 4bits en otro byte, el cual tambien contiene 4bits de otro
	// cluster.
	// Por ejemplo, los clusteres consecutivos ABC DEF se almacenan de la siguiente manera:
	//
	//	BC|FA|DE
	//  0  1  2
	//
	//	Respeta el almacenamiento LITTLE-ENDIAN (el byte menos significavo va primero), y el byte
	//	central es compartido por los dos clusteres. Ademas, el cluster almacenado en una posicion
	//	par (ABC) se almacena distinto que el otro (DEF). Para completar esto, puede que parte del
	//  cluster se almacene en un sector y parte en el siguiente.
	//
	
	if(desp == BPB_LARGO_SECT-1) { // Hay que cargar el otro sector?
		if(!(bp2 = sec_obtener(sect+1))) 
			return 1;
		
		// Marca al bufer como FAT
		bp2->flags &= ~SEC_DATOS;
		bp2->flags |= SEC_FAT;
	
		// El byte mas significativo se encuentra en el bufer2.
		hip = &(bp2->buf[0]);
	}
	
	if(clust2 == LEER_CLUSTER) {	// Lee el cluster
		if(clust & 0x01) 	// impar
			return MKWORD(*hip, *lop) >> 4;
		else				// par
			return MKWORD(*hip, *lop) & 0x0FFF;
	}
	else {							// Modifica el cluster
		if(clust & 0x01) { 	// impar
			*lop = LOBYTE(clust2) & (*lop & 0x0F);
			*hip = HIBYTE(clust2);
		}
		else {				// par
			*lop = LOBYTE(clust2);
			*hip = HIBYTE(clust2) & (*hip & 0xF0);
		}
		
		// Marca los buferes como modificados.
		bp->flags |= SEC_MODIF;
		if(bp2) bp2->flags |= SEC_MODIF;
		
		return clust2;
	}

}

//
//	FATPROX
//	Devuelve el cluster que le sigue en la cadena de clusteres en la FAT.
//
//	clust		Cluster a partir del cual se pretende seguir la cadena
//
//	DEVUELVE: El valor del proximo cluster en la cadena
//
//	ERROR:
//		1	en caso de error.
WORD fatprox(WORD clust) {
	WORD candidato, siguiente;
	
	candidato = fatclust(clust, LEER_CLUSTER);
	
	if (candidato < 2 || candidato >= CLUST_MALO)
		return candidato;
	
	//	Verifica que no haya clusteres invalidos o libres en la cadena, lo cual indica que la 
	//	cadena esta corrompida
	siguiente = fatclust(candidato, LEER_CLUSTER);
	if (siguiente < 2 || (siguiente < CLUST_MALO && siguiente > CLUSTER_MAX))
	{
		return 1; // Error, la cadena esta mal formada
	}
	
	return candidato;
	
}

//	FATBORRAR
//	Deja libre toda la cadena de clusteres que comienza con el cluster dado.
//
//
CODERR fatborrar(WORD clust) {
	
	WORD prox;
	
	if(clust < 2 || clust > CLUSTER_MAX)
		return E_ARGINVAL;
		
	while(clust > 2 && clust < CLUSTER_MAX) {
		
		prox = fatprox(clust);			// Obtiene el proximo cluster
		if(prox == 1) 					// Si la cadena de clusteres es erronea termina aca
			return E_ERROR;
			
		if(fatclust(clust, CLUST_LIBRE) == 1) // Libera el cluster actual
			return E_ERROR;;	

		clust = prox;					// Prosigue con el siguiente cluster.
	}
	
	return E_EXITO;
	
}

//
//	FATLIBRE
//	Devuelve un cluster libre.
//
//	DEVUELVE: El primer cluster libre en la fat.
//
//	ERROR:
//		Devuelve 1 cuando no hay clusteres libres, es decir, el disco esta lleno.
//
WORD fatlibre(void) {
	
	WORD clust;
	WORD val;
	
	for(clust = 2; clust <= CLUST_MAX; clust++) {
		if(fatclust(clust, LEER_CLUSTER) == CLUST_LIBRE)
			return clust;
	}
	
	return 1;
}

//
//	RUTA_A_NOMBRE83
//	Obtiene el nombre de un archivo de una ruta y lo transforma al formato 8.3
//
//	nombre83	Donde se guardara el nombre 8.3 del archivo
//	ruta		Una ruta de archivo bien formada
//
//	DEVUELVE:	El puntero al separador '/' que sigue o al final de la cadena.
//
const char* ruta_a_nombre83(char* nombre83, const char* ruta) {
	int i;
	memset(nombre83, ' ', ENT_NOMBRE_SZ + ENT_EXT_SZ);

	for (i = 0; i < ENT_NOMBRE_SZ + ENT_EXT_SZ; i++, ruta++)
	{
		char c = *ruta;
		if (c == '.')
			i = ENT_NOMBRE_SZ - 1;
		else if (c != '\0' && c != '/')
			nombre83[i] = c;
		else
			break;
	}
	return ruta;
}

//
//	FAT_BPBBUENO
//	Corrobora si la informacion en el Bios Parameter Block del sector 0 coincide con la del tipo
// de unidad que SOM 8086 puede manejar.
//
BOOL fat_bpbbueno(void) {
	
	SECBUF* bp;
	BPB* bpb;
	
	if(!(bp=sec_obtener(0)))
		return FALSE;
	
	bpb = (BPB*)bp->buf;
	
	if(bpb->largo_sect 	!= BPB_LARGO_SECT) 	return FALSE;
	if(bpb->sect_p_clust!= BPB_SECT_P_CLUST)return FALSE;
	if(bpb->sect_reserv != BPB_SECT_RESERV) return FALSE;
	if(bpb->n_fats 		!= BPB_N_FATS) 		return FALSE;
	if(bpb->ent_raiz 	!= BPB_ENT_RAIZ) 	return FALSE;
	if(bpb->media 		!= BPB_MEDIA) 		return FALSE;
	if(bpb->sect_p_fat 	!= BPB_SECT_P_FAT) 	return FALSE;
	if(bpb->sect_p_cil 	!= BPB_SECT_P_CIL) 	return FALSE;
	if(bpb->n_cabezales != BPB_N_CABEZALES) return FALSE;
	if(bpb->extendido 	!= BPB_EXTENDIDO) 	return FALSE;

	return TRUE;
	
}
