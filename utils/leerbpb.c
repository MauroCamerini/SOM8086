#include <stdio.h>
#include <stdlib.h>

#define TAMANO_BPB 0x3e

static char bpb[TAMANO_BPB];

int main(int argc, char** argv) {


	FILE* 	archivo;
	size_t 	bytes_leidos;
	
	if(argc != 2) {
		printf("LEERBPB - Leer Bios Parameter Block\n");
		printf("\nMuestra el bloque de parametros para el BIOS de una imagen de disco floppy.");
		printf("Modo de uso:\n");
		printf("\t%s floppy.img\n", argv[0]);
		exit(1);
	}
	
	archivo = fopen(argv[1], "rb");
	if(!archivo){
		printf("No se pudo abrir el archivo %s\n", argv[1]);
		exit(1);
	}
	
	bytes_leidos = fread(bpb,1,TAMANO_BPB,archivo);
	if(bytes_leidos < TAMANO_BPB) {
		printf("Archivo invalido.\n");
		exit(1);
	}
	
	fclose(archivo);
	
	printf("JMP y NOP:           %hhx %hhx %hhx\n", bpb[0], bpb[1], bpb[2]);
	printf("Sistema operativo:   %.8s\n", bpb+0x03);
	printf("Bytes por sector:    %hu\n", (unsigned short)*(bpb+0x0b));
	printf("Sector por cluster:  %hhu\n", (unsigned char)*(bpb+0x0d));
	
	printf("Sectores reservados: %hhu\n", (unsigned char)*(bpb+0x0e));
	printf("Cantidad de FATs:    %hhu\n", (unsigned char)*(bpb+0x10));

	printf("Entradas dir. raíz:  %hu\n", (unsigned short)*(bpb+0x11));
	printf("N de secto res:      %hu\n", (unsigned short)*(bpb+0x13));

	printf("Tipo de unidad:      %hhx\n", (unsigned char)*(bpb+0x15));

	printf("Sectores por FAT:    %hu\n", (unsigned short)*(bpb+0x16));
	printf("Sectores por track:  %hu\n", (unsigned short)*(bpb+0x18));
	
	printf("Lados:               %hu\n", (unsigned short)*(bpb+0x1a));
	printf("Sectores ocultos:    %hu\n", (unsigned short)*(bpb+0x1c));
	
	printf("N de sectores 32b:   %u\n", (unsigned)*(bpb+0x20));
	
	printf("N de unidad:         %hhu\n", (unsigned char)*(bpb+0x24));

	printf("BPB Extendido:       %hhx\n", (unsigned char)*(bpb+0x26));
	printf("Nº de serie:         %x\n", (unsigned)*(bpb+0x27));
	printf("Etiqueta:            %.11s\n", bpb+0x2b);
	printf("Sistema de archivos: %.8s\n", bpb+0x36);

	return 0;

}
