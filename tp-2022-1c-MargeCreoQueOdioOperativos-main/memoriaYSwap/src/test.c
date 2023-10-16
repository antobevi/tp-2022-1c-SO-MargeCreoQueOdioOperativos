#include "test.h"

void correr_test(){

	/*Llamar las funciones de los test aca*/
	iniciar_procesos_en_memoria();
	verificar_procesos_tabla();
	leer_tabla();
}

void iniciar_procesos_en_memoria(){
	int tamanio = 64;
	printf("\n nro_tabla nvl1 del proceso 1 %i",iniciar_proceso_en_memoria(0,tamanio*6));//nivl1:0 nvl2:0 y 1
	devolver_marco(0,0,0);//0
	devolver_marco(0,0,1);//1
	devolver_marco(0,0,2);//2
	devolver_marco(0,0,1);//1
	devolver_marco(0,1,1);//5
	devolver_marco(0,1,0);//4
	devolver_marco(0,0,3);//3

//	Tabla_nvl1* proceso = (Tabla_nvl1*)list_get(tabla_de_paginas,0);
/*
	void* escritura = malloc(TAM_PAGINA);
	uint32_t valor=1;
	memcpy(escritura,&valor,sizeof(uint32_t));
	valor=3;
	memcpy(escritura+(TAM_PAGINA-4),&valor,sizeof(uint32_t));
	valor =0;
	escribir_en_swap(proceso->archivo_swap,tamanio*6,0,escritura);
	free(escritura);
	escritura = lectura_de_swap(proceso->archivo_swap,0,tamanio*6);
	memcpy(&valor,escritura,sizeof(uint32_t));
	printf("Escribio: %i ",valor);
	memcpy(&valor,escritura+(TAM_PAGINA-4),sizeof(uint32_t));
	printf("Escribio: %i ",valor);
*/
//	printf("\n nro_tabla nvl1 del proceso 2 %i",iniciar_proceso_en_memoria(2,tamanio*4));//nivl1:0 nvl2:2
//	printf("\n nro_tabla nvl1 del proceso 3 %i",iniciar_proceso_en_memoria(3,tamanio*1));//nivl1:0 nvl2:3
//	printf("\n nro_tabla nvl1 del proceso 4 %i",iniciar_proceso_en_memoria(4,tamanio*2));//nivl1:1 nvl2:4
//	printf("\n nro_tabla nvl1 del proceso 5 %i",iniciar_proceso_en_memoria(5,tamanio*4));//nivl1:1 nvl2:5
//	printf("\n nro_tabla nvl1 del proceso 6 %i",iniciar_proceso_en_memoria(6,tamanio*4));//nivl1:1 nvl2:6
//	printf("\n nro_tabla nvl1 del proceso 7 %i",iniciar_proceso_en_memoria(7,tamanio*5*5));//nivl1:2 nvl2:7
//	printf("\n nro_tabla nvl1 del proceso 8 %i",iniciar_proceso_en_memoria(8,tamanio*5));//nivl1:3 nvl2:8 y 9
//	printf("\n nro_tabla nvl1 del proceso 9 %i",iniciar_proceso_en_memoria(9,tamanio*2));//nivl1:1 nvl2:10
//	printf("\n nro_tabla nvl1 del proceso 10 %i",iniciar_proceso_en_memoria(10,tamanio*1));//nivl1:3 nvl2:11


}

void verificar_procesos_tabla(){

}
