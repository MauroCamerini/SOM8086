#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#define BPB_CORRECTO bios_parameter_block_correcto
#define BPB_DESP	 11		// Desplazamiento del BPB en el sector de arranque
#define BPB_TAM		 28 	// Tamano del BPB (en realidad de los campos que se desea verificar)
#define NUCLEO_DESP	 0x1F3 	// Ubicacion del nombre de archivo en el sector de arranque
char bios_parameter_block_correcto[] = 
{
	0x00, 0x02, 0x01, 0x01,	0x00, 0x02, 0xE0, 0x00, 0x40, 0x0B, 0xF0, 0x09, 0x00, 0x12, 
	0x00, 0x02,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x29
};


void mostrar_ayuda() {
	puts(
"\
CDISCM\
	Crea una imagen de disquete que contiene el programa de arranque y el nucleo del \
	Sistema Opertivo M 8086. \
\
Modo de uso:\
	cdiscm imagen arranque nucleo\
	\
	imagen		Nombre del archivo de imagen de disco que se va a crear. Cuidado, si ya existe\
				lo sobrescribe.\
				\
	arranque 	Archivo que contiene el programa de arranque. Debe tener como minimo 512 bytes.\
				Si tiene mas, se consideran solo los primero 512. Se verifica que los datos del\
				bios paramater block sean correctos y que posea la marca AA55 al final. Si esa\
				informacion no es correcta la operacion fracasa.\
				De la direccion 0x01F3 del sector de arranque se obtiene el nombre que tendra el\
				archivo del nucleo dentro de la imagen de disco.\
				\
	nucleo		Archivo que contiene el nucleo del sistema operativo. Se verifica que no supere los\
				29kb. Si lo supera la operacion fracasa.\
"
	);

}

int main() {
	
	
}
