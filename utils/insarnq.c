/*
	INSARNQ - Instalar arranque
	
	Copia los primeros 512 bytes de un archivo en otro.
	Es utilizado para instalar un sector de arranque en una imagen de disco floppy.

*/

#include <stdio.h>
#include <stdlib.h>

static char bufer[512];

int main(int argc, char** argv)
{
    FILE *archivo;
	size_t bytes_leidos;

	//
	//	Verifica la cantidad de argumentos pasados por la linea de comandos
	//
    if(argc == 1) {
        printf("INSARNQ - Instalar Arranque\n");
        printf("Copia los primeros 512 bytes de un archivo en otro.\n");
        printf("Es utilizado para instalar un sector de arranque en una imagen de disco floppy.\n");
		printf("\n");
        printf("Modo de uso:\n");
        printf("INSARNQ imagen arranque\n");
        printf("\timagen   - imagen de archivo floppy\n");
        printf("\tarranque - archivo de hasta 512bytes con el programa de arranque\n");
        exit(1);
    }
    else if (argc > 3) {
        printf("ERROR. Demasiados argumentos.\n");
        exit(1);
    }
	
	//
	// Carga los primeros 512 bytes del segundo archivo.
	//
    archivo = fopen(argv[2], "rb");
    if(!archivo) {
        printf("No se pudo abrir el archivo %s.\n", argv[2]);
		exit(1);
    }
	
	bytes_leidos = fread((void*)bufer, sizeof(char), 512, archivo);

	if(ferror(archivo)) {
		printf("No se pudo leer el archivo %s.\n", argv[2]);
		exit(1);
	}
	
	printf("%i bytes leidos del archivo %s.\n", (int)bytes_leidos, argv[2]);
	
	fclose(archivo);
	
	//
	// Escribe los bytes leidos en el primer archivo.
	//
	archivo = fopen(argv[1], "rb+");
	if(!archivo) {
        printf("No se pudo abrir el archivo %s.\n", argv[1]);
		exit(1);
    }
	
	rewind(archivo);
	fwrite(bufer, sizeof(char), bytes_leidos, archivo);
	
	if(ferror(archivo)) {
		printf("No se pudo escribir el archivo %s.\n", argv[1]);
		exit(1);
	}
	
	printf("Archivo %s modificado con exito.\n", argv[1]);
	
	fclose(archivo);
	
    return 0;
}
