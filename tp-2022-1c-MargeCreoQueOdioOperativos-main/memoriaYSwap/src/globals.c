#include "globals.h"

int static cg_tabla_nvl1;

void inicializar_config(char* nombre_config){


	char*RUTA_CONFIG =malloc(strlen("../Configs/")+1);
	strcpy(RUTA_CONFIG,"../Configs/");
	string_append(&RUTA_CONFIG,nombre_config);

	config = config_create(RUTA_CONFIG);
	free(RUTA_CONFIG);

	char* algoritmo = config_get_string_value(config,"ALGORITMO_REEMPLAZO");
	ALGORITMO_REEMPLAZO = malloc(strlen(algoritmo)+1);
	strcpy(ALGORITMO_REEMPLAZO,algoritmo);

	char* path = config_get_string_value(config,"PATH_SWAP");
	PATH_SWAP= malloc(strlen(path)+1);
	strcpy(PATH_SWAP,path);

	PUERTO = config_get_int_value(config,"PUERTO_ESCUCHA");
	TAM_MEMORIA= config_get_int_value(config,"TAM_MEMORIA");
	TAM_PAGINA= config_get_int_value(config,"TAM_PAGINA");
	ENTRADAS_POR_TABLA= config_get_int_value(config,"ENTRADAS_POR_TABLA");
	RETARDO_MEMORIA= config_get_int_value(config,"RETARDO_MEMORIA");
	RETARDO_SWAP = config_get_int_value(config,"RETARDO_SWAP");
	MARCOS_POR_PROCESO =config_get_int_value(config,"MARCOS_POR_PROCESO");
	RETARDO_MEMORIA = RETARDO_MEMORIA/1000;

	remove("../memoriaYSwap.log");
	remove("../tester_memoria.log");
	remove("../servidor.log");
	log_global = log_create("../memoriaYSwap.log", "LOG-GLOBAL-MEMORIA", 1, LOG_LEVEL_TRACE);
	log_tester = log_create("../tester_memoria.log", "LOG-GLOBAL-MEMORIA", 1, LOG_LEVEL_TRACE);
	servidor = log_create("../servidor.log","Log server",0,LOG_LEVEL_TRACE);
	tabla_de_paginas =list_create();
	marcos_disp = list_create();
	cg_tabla_nvl2 = 0;
	cg_tabla_nvl1 = 0;

	mkdir(PATH_SWAP, 0700);
	log_info(log_global,"Directorio creado");
}

void borrar_logs(char*logs){
	int borrar = remove(logs);

	if (borrar == 0){
		log_info(log_global,"Se elimino el archivo: %s/n",logs);
		//log_info(log_global,"Se elimino el archivo: %s/n",nombre_completo);
	}else{
		log_error(log_global,"No se pudo eliminar el archivo: %s/n",logs);
		//log_info(log_global,"No se pudo eliminar el archivo: %s/n",nombre_completo);
	}
}

void iniciar_tabla_marcos_disponibles(){

	for(uint32_t marco = 0; marco<(TAM_MEMORIA/TAM_PAGINA); marco++){//recorre la lista de lvl 1

		liberar_marco(marco);

	}
	log_info(log_global,"Marcos totales en memoria = %i \n",list_size(marcos_disp));



}

void liberar_marco(uint32_t marco_disp){

	Marco* marco = malloc(sizeof(Marco));
	marco->nro_marco_disp = marco_disp;
	list_add(marcos_disp,marco);
}

uint32_t obtener_marco_disp(){
	Marco* marco = list_remove(marcos_disp,0);
	uint32_t marco_disp = marco->nro_marco_disp;
	destruir_marco(marco);

	return marco_disp;
}
Tabla_nvl1* instanciar_tabla_nvl1(uint32_t proceso_id){

	Tabla_nvl1* tabla_nvl1= malloc(sizeof(Tabla_nvl1));

	tabla_nvl1->nro_tabla_nvl1= cg_tabla_nvl1;
	tabla_nvl1->proceso_id= proceso_id;
	tabla_nvl1->Entradas_tabla_nvl1 = list_create();
	tabla_nvl1->lista_aux = list_create();

	cg_tabla_nvl1++;

	return tabla_nvl1;
}

Entrada_tabla_nvl1* instanciar_entrada_nvl1(){

	Entrada_tabla_nvl1* entrada_tabla_nvl1= malloc(sizeof(Entrada_tabla_nvl1));
	entrada_tabla_nvl1->nro_tabla_nvl2 = cg_tabla_nvl2;

	entrada_tabla_nvl1->entradas_tabla_nvl2= list_create();

	cg_tabla_nvl2++;

	return entrada_tabla_nvl1;
}

Entrada_tabla_nvl2* instanciar_entradas_nvl2(uint32_t nro_pagina){

	Entrada_tabla_nvl2* fila_tabla = malloc(sizeof(Entrada_tabla_nvl2));

	fila_tabla->nro_marco = 0;
	fila_tabla->nro_pagina = nro_pagina;
	fila_tabla->bit_uso = 0;
	fila_tabla->bit_presencia= 0;
	fila_tabla->bit_modificado = 0;


	return fila_tabla;
}

void destruir_Entrada_tabla_nvl2(Entrada_tabla_nvl2* fila){
	free(fila);
}

void destruir_tablas_nvl2(Entrada_tabla_nvl1* nodo2){
	list_destroy_and_destroy_elements(nodo2->entradas_tabla_nvl2,(void*)destruir_Entrada_tabla_nvl2);
	free(nodo2);
}
void destruir_list_aux(Remplazo_paginas* nodo){
	free(nodo);
}
void destruir_tablas_nvl1(Tabla_nvl1* nodo){
	list_destroy_and_destroy_elements(nodo->Entradas_tabla_nvl1,(void*)destruir_tablas_nvl2);
	//list_destroy_and_destroy_elements(nodo->lista_aux,(void*)destruir_list_aux);
	list_destroy(nodo->lista_aux);
	free(nodo->archivo_swap);
	free(nodo);
}


void destruir_marco(Marco* marco){
	free(marco);
}
void liberar_memoria(){
	free(IP);
	free(ALGORITMO_REEMPLAZO);
	free(PATH_SWAP);
	free(espacio_usuario);
	log_destroy(log_global);
	log_destroy(log_tester);
	config_destroy(config);
	list_destroy_and_destroy_elements(marcos_disp,(void*)destruir_marco);
	list_destroy_and_destroy_elements(tabla_de_paginas,(void*)destruir_tablas_nvl1);
}

void leer_tabla(){
	t_list_iterator* tabla = list_iterator_create(tabla_de_paginas);

	printf("\n tamanio tabla de paginas: %i",list_size(tabla_de_paginas));
	while(list_iterator_has_next(tabla)){
		Tabla_nvl1* tabla1 = list_iterator_next(tabla);
		t_list_iterator* tabla2 = list_iterator_create(tabla1->Entradas_tabla_nvl1);
		printf("\033[0;34m");
		printf("\n ----NRO TABLA NVL1:%i---- \n",tabla1->nro_tabla_nvl1);
		printf("PROCESO: %i \n",tabla1->proceso_id);

		while(list_iterator_has_next(tabla2)){
			Entrada_tabla_nvl1* nodo_tabla_nvl2 = list_iterator_next(tabla2);
			printf("\033[0;31m");
			printf("NUMERO TABLA DE NIVEL 2: %i \n",nodo_tabla_nvl2->nro_tabla_nvl2);

			printf("cantidad de entradas: %i \n\n",list_size(nodo_tabla_nvl2->entradas_tabla_nvl2));
			t_list_iterator* filas = list_iterator_create(nodo_tabla_nvl2->entradas_tabla_nvl2);
			while(list_iterator_has_next(filas)){
				Entrada_tabla_nvl2* fila = list_iterator_next(filas);
				printf("\033[0;37m");
				printf("\n		nro pagina: %i \n",fila->nro_pagina);
				printf("		nro marco: %i \n",fila->nro_marco);
				printf("		bit presencia: %i \n",fila->bit_presencia);
				printf("		bit modificado: %i \n",fila->bit_modificado);
				printf("		bit uso: %i \n",fila->bit_uso);
			}
			list_iterator_destroy(filas);
		}
		list_iterator_destroy(tabla2);
		t_list_iterator* list_auxiliar = list_iterator_create(tabla1->lista_aux);
		while(list_iterator_has_next(list_auxiliar)){
			Remplazo_paginas* remplazo = list_iterator_next(list_auxiliar);

			printf("paginas en memoria: %i",remplazo->pag_en_memoria);
		}
		list_iterator_destroy(list_auxiliar);
	}
	list_iterator_destroy(tabla);

}

void leer_tabla_aux(){

	t_list_iterator* tabla = list_iterator_create(tabla_de_paginas);

	while(list_iterator_has_next(tabla)){
		Tabla_nvl1* tabla1 = list_iterator_next(tabla);
		t_list_iterator* list_auxiliar = list_iterator_create(tabla1->lista_aux);

	printf("\n Tabla de paginas de proceso:%i \n ",tabla1->proceso_id);
		while(list_iterator_has_next(list_auxiliar)){
			Remplazo_paginas* remplazo = list_iterator_next(list_auxiliar);

			printf("paginas en memoria: %i \n",remplazo->pag_en_memoria);
		}
		list_iterator_destroy(list_auxiliar);
	}
	list_iterator_destroy(tabla);

}
