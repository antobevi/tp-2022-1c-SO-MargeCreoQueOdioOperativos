#include "swap.h"

#ifndef archivos_h
#define archivos_h



void crear_archivos(char* archivo_swap,int tam_archivo){
	sleep(RETARDO_SWAP/1000);

	int fd;
	FILE* f = fopen(archivo_swap,"wb+");


	if (f == NULL){
		log_error(log_global,"Eror al crear el archivo");
	}else{
		creat(archivo_swap,0644);
		fd = open(archivo_swap,O_RDWR);
		ftruncate(fd,tam_archivo);
		if(fd == -1){

			log_error(log_global,"Eror al crear el archivo");

		}else{
			log_info(log_global,"Se creo el archivo");
		}


	}
	close(fd);

}


//void escribir_archivo(voidinfo,uint32_t nro_archivo,uint32_t marco,uint32_t proceso,uint32_t pagina){//agrego nombre, 1 dir de memoria uint32 y contenido
void escribir_en_swap(char* nombre_archivo,uint32_t tamanio_swap,uint32_t pagina,void* contenido_pagina){

	sleep(RETARDO_SWAP/1000);

	int fd = open(nombre_archivo,O_RDWR);
	char* addr = mmap(NULL,tamanio_swap,PROT_WRITE,MAP_SHARED,fd,0);
    off_t desplazamiento = pagina*TAM_PAGINA;
	memcpy(addr+desplazamiento,contenido_pagina, TAM_PAGINA);

//	close(fd);
	log_info(log_global,"La pagina %i ya esta en swap",pagina);
}

void* lectura_de_swap(char* nombre_archivo ,uint32_t pagina, uint32_t tamanio_swap){
	sleep(RETARDO_SWAP/1000);

	void* info = malloc(TAM_PAGINA);
    off_t desplazamiento = pagina*TAM_PAGINA;

	int fd = open(nombre_archivo,O_RDWR);
	log_info(log_global,"Pagina: %i leida de swap",pagina);
	char *addr = mmap(NULL,tamanio_swap,PROT_WRITE,MAP_SHARED,fd,0);

	memcpy(info, addr + desplazamiento, TAM_PAGINA);

//	close(fd);
	return info;
}

void borrar_archivo(char* archivo_swap){ //char* directorio? probar si lo reconoce
	sleep(RETARDO_SWAP/1000);

     int borrar = remove(archivo_swap);

    if (borrar == 0){
        log_info(log_global,"Se elimino el archivo: %s /n",archivo_swap);
        //log_info(log_global,"Se elimino el archivo: %s/n",nombre_completo);
    }else{
        log_error(log_global,"No se pudo eliminar el archivo: %s/n",archivo_swap);
        //log_info(log_global,"No se pudo eliminar el archivo: %s/n",nombre_completo);
    }
}

#endif /* ARCHIVOSH */
