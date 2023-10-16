/*
 * memoria.c
 *
 *  Created on: 25 abr. 2022
 *      Author: utnso
 */
#include "memoria.h"

uint32_t iniciar_proceso_en_memoria(uint32_t proceso_id,uint32_t tamanio_p){


	uint32_t cantidad_de_paginas;
	cg_tabla_nvl2= 0;

	if(tamanio_p % TAM_PAGINA !=0) {
		cantidad_de_paginas = (uint32_t) (tamanio_p/TAM_PAGINA) +1;
	}
	else {
		cantidad_de_paginas = tamanio_p/TAM_PAGINA;
	}

	uint32_t cant_tab_nvl2 = tablas_de_nvl2_a_usar(cantidad_de_paginas);

	Tabla_nvl1* tabla_nvl1 = instanciar_tabla_nvl1(proceso_id);

	uint32_t i=0;

	int pagina = 0;

	while( i< cant_tab_nvl2 ){

		int j = 0;

		Entrada_tabla_nvl1* entrada_nvl1 = instanciar_entrada_nvl1();

		while(j < ENTRADAS_POR_TABLA && pagina<cantidad_de_paginas){

			list_add(entrada_nvl1->entradas_tabla_nvl2,instanciar_entradas_nvl2(pagina));

			j++;
			pagina++;
		}

		list_add(tabla_nvl1->Entradas_tabla_nvl1,entrada_nvl1);
		i++;

	}

	cg_tabla_nvl2 = 0;

	char*nombre_archivo = malloc(strlen(PATH_SWAP)+1);//+sizeof(char)*strlen(".swap")+sizeof(char)*strlen(string_itoa(proceso_id)));
	strcpy(nombre_archivo,PATH_SWAP);
	string_append(&nombre_archivo,"/");
	char*nombre_nro_pagina = string_itoa(proceso_id);
	string_append(&nombre_archivo,nombre_nro_pagina);
	string_append(&nombre_archivo,".swap");
	free(nombre_nro_pagina);

	tabla_nvl1->archivo_swap = nombre_archivo;
	tabla_nvl1->tam_archivo = cantidad_de_paginas*TAM_PAGINA;
	tabla_nvl1->tam_proceso = tamanio_p;


	crear_archivos(nombre_archivo,cantidad_de_paginas*TAM_PAGINA);

	//log_warning(log_tester,"PROCESOOOOOOOOOOOOOOOOOOO: %i, NOMBRE: %s",proceso_id,tabla_nvl1->archivo_swap);
	log_info(log_global,"Numero de tabla asignada: %i \n",tabla_nvl1->nro_tabla_nvl1);
	list_add(tabla_de_paginas,tabla_nvl1);
	return (list_size(tabla_de_paginas)-1);
}

void suspender_proceso_en_memoria(uint32_t proceso_id){//TODO:TESTEAR

	swapear_proceso(proceso_id);
	eliminar_proceso_de_eu(proceso_id);
	suspender_de_tabla(proceso_id);
}


void finalizar_proceso(uint32_t proceso_id){
	eliminar_proceso_de_eu(proceso_id);
	borrar_archivo_de_memoria(proceso_id);
	suspender_de_tabla(proceso_id);
}


//entrada_tabla_segundo_nivel es la pagina
uint32_t devolver_marco(uint32_t id_tabla_nivel_1,uint32_t entrada_tabla_primer_nivel,uint32_t entrada_tabla_segundo_nivel){
	Tabla_nvl1* tabla_nivel_1 = (Tabla_nvl1*)list_get(tabla_de_paginas,id_tabla_nivel_1);
	uint32_t proceso_id = tabla_nivel_1->proceso_id;
	Entrada_tabla_nvl1* entrada_tabla_nivel_1 = (Entrada_tabla_nvl1*)list_get(tabla_nivel_1->Entradas_tabla_nvl1,entrada_tabla_primer_nivel);
	Entrada_tabla_nvl2* entrada_tabla_nivel_2 = (Entrada_tabla_nvl2*)list_get(entrada_tabla_nivel_1->entradas_tabla_nvl2,entrada_tabla_segundo_nivel);



	uint32_t pagina = entrada_tabla_nivel_2->nro_pagina;

	log_info(log_global, "Se accedio a la pagina %i \n", pagina);


	//Al ser solo remplazo, solo toco el bit de uso
	if(esta_en_memoria(proceso_id,pagina)){
		log_error(log_global,"ENTRO ACA");
		entrada_tabla_nivel_2->bit_uso =1;
		hubo_remplazo = NO_REEMPLAZO;
	}else if(MARCOS_POR_PROCESO>marcos_ocupados(proceso_id)){

		Remplazo_paginas* remplazo = malloc(sizeof(Remplazo_paginas));
		remplazo->pag_en_memoria = pagina;

		if(list_size(tabla_nivel_1->lista_aux)==0){
			tabla_nivel_1->puntero = pagina;
		}
		list_add(tabla_nivel_1->lista_aux,remplazo);

		entrada_tabla_nivel_2->bit_uso = 1;
		entrada_tabla_nivel_2->bit_presencia = 1;
		entrada_tabla_nivel_2->nro_marco = obtener_marco_disp();
		traer_pagina_a_eu(tabla_nivel_1,entrada_tabla_nivel_2->nro_pagina,entrada_tabla_nivel_2->nro_marco);
		hubo_remplazo = NO_REEMPLAZO;
	}else if(string_equals_ignore_case("CLOCK",ALGORITMO_REEMPLAZO)){
		hubo_remplazo = REEMPLAZO;

		clock_n(proceso_id,pagina);

	}else if(string_equals_ignore_case(ALGORITMO_REEMPLAZO,"CLOCK-M")){
		hubo_remplazo = REEMPLAZO;
		clock_m(proceso_id,pagina);

	}
	//	log_info(log_global,"El puntero se encuentra en la pagina: %i \n",tabla_nivel_1->puntero);
	//leer_tabla_aux();
	return entrada_tabla_nivel_2->nro_marco;
}

/////////////////////////       FUNCIONES AUXILIARES

void marcos_disponibles(Tabla_nvl1* tabla1,uint32_t pagina,Entrada_tabla_nvl2* nodo_entrada_2){
	Remplazo_paginas* remplazo = malloc(sizeof(Remplazo_paginas));
	remplazo->pag_en_memoria = pagina;

	if(list_size(tabla1->lista_aux)==0){
		tabla1->puntero = pagina;
	}
	list_add(tabla1->lista_aux,remplazo);

	nodo_entrada_2->bit_uso = 1;
	nodo_entrada_2->bit_presencia = 1;
	nodo_entrada_2->nro_marco = obtener_marco_disp();
	traer_pagina_a_eu(tabla1,nodo_entrada_2->nro_pagina,nodo_entrada_2->nro_marco);
	//traer de swap la pagina
}

void clock_n(uint32_t proceso_id,uint32_t pagina){


	remplazo_clock(proceso_id,pagina);
	modificar_tabla(proceso_id,pagina);

}

void modificar_tabla(uint32_t proceso_id, uint32_t pagina){

	Tabla_nvl1* tabla_nvl1 = buscar_proceso_tabla1(proceso_id);
	uint32_t entrada_1 = pagina / ENTRADAS_POR_TABLA;
	uint32_t entrada_2 = pagina % ENTRADAS_POR_TABLA;

	Entrada_tabla_nvl1* entrada_tabla_1 = (Entrada_tabla_nvl1*)list_get(tabla_nvl1->Entradas_tabla_nvl1,entrada_1);
	Entrada_tabla_nvl2* nodo = (Entrada_tabla_nvl2*)list_get(entrada_tabla_1->entradas_tabla_nvl2,entrada_2);
	nodo->bit_presencia = 1;
	nodo->bit_uso = 1;
	nodo->nro_marco = obtener_marco_disp();
	//printf("pagina: %i",nodo->nro_pagina);

	//traer de swap la pagina
	traer_pagina_a_eu(tabla_nvl1,nodo->nro_pagina,nodo->nro_marco);


}
uint32_t remplazo_clock(uint32_t proceso_id,uint32_t pagina_a_remplazar){

	Tabla_nvl1* tabla_nvl1 = buscar_proceso_tabla1(proceso_id);
	int i= 0;
	int j=0;
	bool encontro_remplazo = false;
	uint32_t pag_r = 0;
	while(i<MARCOS_POR_PROCESO){

		Remplazo_paginas* remplazo = (Remplazo_paginas*)list_get(tabla_nvl1->lista_aux,i);
		j=i;

		if(tabla_nvl1->puntero == remplazo->pag_en_memoria){


			while(!encontro_remplazo){

				if(j==MARCOS_POR_PROCESO){
					j=0;
				}

				remplazo = (Remplazo_paginas*)list_get(tabla_nvl1->lista_aux,j);

				uint32_t pagina = remplazo->pag_en_memoria;
				//printf("REMPLAZO: %i \n",pagina);
				uint32_t entrada_1 = pagina / ENTRADAS_POR_TABLA;
				uint32_t entrada_2 = pagina % ENTRADAS_POR_TABLA;

				Entrada_tabla_nvl1* entrada_tabla_1 = (Entrada_tabla_nvl1*)list_get(tabla_nvl1->Entradas_tabla_nvl1,entrada_1);
				Entrada_tabla_nvl2* nodo = (Entrada_tabla_nvl2*)list_get(entrada_tabla_1->entradas_tabla_nvl2,entrada_2);


				if(nodo->bit_uso == 0){//remplazo pagina

					remplazo_encotrado(tabla_nvl1,nodo,remplazo,pagina_a_remplazar,proceso_id,j);
					encontro_remplazo = true;
					//printf("VICTIMA : %i \n",pagina);

				}else{
					//cambio el bit de uso
					nodo->bit_uso = 0;

				}

				j++;
			}
			if(encontro_remplazo){
				break;
			}
		}
		i++;
	}

	return pag_r;
}

void clock_m(uint32_t proceso_id,uint32_t pagina_nueva){


	Tabla_nvl1* tabla_1 = buscar_proceso_tabla1(proceso_id);
	int i = 0;
	int j = 0;
	bool remplazo_encontrado = false;
	bool primera_vuelta = true;

	while(i<list_size(tabla_1->lista_aux)){//recorro la lista hasta encontrar el puntero

		Remplazo_paginas* remplazo = (Remplazo_paginas*)list_get(tabla_1->lista_aux,i);

		if(remplazo->pag_en_memoria == tabla_1->puntero){//lo encuentro

			j=i;
			int vuelta = 0;

			while(!remplazo_encontrado){//recorro 1 vez
				if(vuelta == MARCOS_POR_PROCESO){//si ya el puntero recorrio toda la vuelta => seteo la vualta a la siguiente
					primera_vuelta = !primera_vuelta;
					vuelta = 0;
				}
				if(j==MARCOS_POR_PROCESO){
					j=0;
				}
				remplazo =(Remplazo_paginas*) list_get(tabla_1->lista_aux,j);

				uint32_t pagina = remplazo->pag_en_memoria;

				uint32_t entrada_1 = pagina / ENTRADAS_POR_TABLA;
				uint32_t entrada_2 = pagina % ENTRADAS_POR_TABLA;
				Entrada_tabla_nvl1* entrada_tabla_1 = (Entrada_tabla_nvl1*)list_get(tabla_1->Entradas_tabla_nvl1,entrada_1);
				Entrada_tabla_nvl2* nodo = (Entrada_tabla_nvl2*)list_get(entrada_tabla_1->entradas_tabla_nvl2,entrada_2);


				//		printf("\n pagina %i \n",pagina);

				if(primera_vuelta){//PRIEMRA VUELTA
					//	printf("\n pagina que entro en la primera vuelta:%i",pagina);
					if(nodo->bit_modificado == 0 && nodo->bit_uso == 0){
						//printf("\n remplaza \n");
						remplazo_encotrado(tabla_1,nodo,remplazo,pagina_nueva,proceso_id,j);
						remplazo_encontrado =  true;
					}

				}else{//SEGUNDA VUELTA

					//printf("\n pagina que entro en la segunda vuelta: %i ",pagina);
					if(nodo->bit_uso == 0){

						remplazo_encotrado(tabla_1,nodo,remplazo,pagina_nueva,proceso_id,j);
						remplazo_encontrado =  true;

					}else{
						nodo->bit_uso = 0;

					}
				}

				j++;
				vuelta++;
			}

		}
		i++;

	}
	modificar_tabla(proceso_id,pagina_nueva);


}

uint32_t remplazo_encotrado(Tabla_nvl1* tabla_1, Entrada_tabla_nvl2* entrada_2,	Remplazo_paginas* remplazo,uint32_t nueva_pagina,uint32_t proceso_id,int j){



	if(entrada_2->bit_modificado == 1){
		log_info(log_global,"Se swappea la pagina: %i  modificada \n ",entrada_2->nro_pagina);
		swapear_pagina(tabla_1,entrada_2->nro_marco,remplazo->pag_en_memoria);
	}

	//Cambio bit precencia de la vieja pagina
	entrada_2->bit_presencia = 0;
	//libero el marco y lo agrego a la lista

	liberar_marco(entrada_2->nro_marco);
	//entrada_2->nro_marco = 0;


	pagina_reemplazada = remplazo->pag_en_memoria;

	//Remplazo la pagina en la lista aux
	remplazo->pag_en_memoria = nueva_pagina;
	//	printf("\n nueva pagina: %i",remplazo->pag_en_memoria);



	//Muevo el puntero a mi siguiente victima
	if(j+1 >= MARCOS_POR_PROCESO){
		j=-1; //si el puntero siguiente es mayor a las entradas de pag, lo reinicio
	}

	Remplazo_paginas* aux = (Remplazo_paginas*)list_get(tabla_1->lista_aux,j+1);

	tabla_1->puntero = aux->pag_en_memoria;

	return 1;
}

void suspender_de_tabla(uint32_t proceso_id){

	Tabla_nvl1* tabla_1 = buscar_proceso_tabla1(proceso_id);

	int i =0;
	while(i<list_size(tabla_1->Entradas_tabla_nvl1)){
		Entrada_tabla_nvl1* entrada_1 = (Entrada_tabla_nvl1*)list_get(tabla_1->Entradas_tabla_nvl1,i);
		int j=0;

		while(j<list_size(entrada_1->entradas_tabla_nvl2)){
			Entrada_tabla_nvl2* entrada_2 = (Entrada_tabla_nvl2*)list_get(entrada_1-> entradas_tabla_nvl2,j);
			if(entrada_2->bit_presencia == 1){

	//log_info(log_tester,"Tabla cam del proceso: %i",tabla_1->proceso_id);
				entrada_2->bit_uso=0;
				liberar_marco(entrada_2->nro_marco);
				entrada_2->bit_presencia = 0;
				entrada_2->bit_modificado =0;
			}
			j++;
		}
		i++;
	}

	list_destroy_and_destroy_elements(tabla_1->lista_aux,(void*)destruir_list_aux);
	tabla_1->lista_aux = list_create();
}



void cambiar_bit_m(uint32_t id_proceso, uint32_t dir_fisica){

	uint32_t marco = dir_fisica/TAM_PAGINA;

	Tabla_nvl1* tabla1 = buscar_proceso_tabla1(id_proceso);

	int i=0;
	while(i< list_size(tabla1->Entradas_tabla_nvl1)){

		Entrada_tabla_nvl1* entrada_tabla_1 = (Entrada_tabla_nvl1*)list_get(tabla1->Entradas_tabla_nvl1,i);

		int j = 0;
		while(j<list_size(entrada_tabla_1->entradas_tabla_nvl2)){

			Entrada_tabla_nvl2* entrada_tabla_2 = (Entrada_tabla_nvl2*)list_get(entrada_tabla_1->entradas_tabla_nvl2,j);
			if(entrada_tabla_2->nro_marco == marco && entrada_tabla_2->bit_presencia ==1){
				log_warning(servidor,"pagina: %i",entrada_tabla_2->nro_pagina);
				entrada_tabla_2->bit_modificado = 1;
			}

			j++;
		}

		i++;
	}
}


uint32_t tablas_de_nvl2_a_usar(uint32_t cant_pagina_proceso){

	uint32_t cantidad_de_tablas;
	if(cant_pagina_proceso % ENTRADAS_POR_TABLA !=0) {
		cantidad_de_tablas = (uint32_t)(cant_pagina_proceso/ENTRADAS_POR_TABLA) +1;
	}
	else {
		cantidad_de_tablas = cant_pagina_proceso/ENTRADAS_POR_TABLA;
	}
	return cantidad_de_tablas;
}




uint32_t marcos_ocupados(uint32_t proceso){

	Tabla_nvl1* tabla =  buscar_proceso_tabla1(proceso);

	return list_size(tabla->lista_aux);
}

void escribir_en_eu(uint32_t dir_fisica, uint32_t valor){
	//Debo guardar el tamaño de lo que voy a leer?, o afirmo que es toda la pagina la que retorno?
	memcpy(espacio_usuario + dir_fisica, &valor, sizeof(uint32_t));//PUEDE CAMBIAR

}

uint32_t leer_en_eu(uint32_t dir_fisica){
	uint32_t valor;

	memcpy(&valor,espacio_usuario+dir_fisica,sizeof(uint32_t));//PUEDE CAMBIAR

	return valor;
}

void eliminar_proceso_de_eu(uint32_t proceso_id){

	Tabla_nvl1* tabla_1 = buscar_proceso_tabla1(proceso_id);
	int i = 0;
	while(i<list_size(tabla_1->Entradas_tabla_nvl1)){
		Entrada_tabla_nvl1* entrada_nvl1 = (Entrada_tabla_nvl1*)list_get(tabla_1->Entradas_tabla_nvl1,i);
		int j = 0;
		while(j<list_size(entrada_nvl1->entradas_tabla_nvl2)){
			Entrada_tabla_nvl2* entrada_nvl2 = (Entrada_tabla_nvl2*)list_get(entrada_nvl1->entradas_tabla_nvl2,j);

			if(entrada_nvl2->bit_presencia == 1){
				//log_info(log_tester,"La pagina: %i que se encuentra en el marco:%i ",entrada_nvl2->nro_pagina, entrada_nvl2->nro_marco);
				void* pagina_vacia = malloc(TAM_PAGINA);//TODO: PUEDE ROMPER POR LIBERAR LA MEMORIA
				escribir_pagina_eu(entrada_nvl2->nro_marco,pagina_vacia);
				//log_info(log_tester,"Pagina:%i de espacio de usuario borrada",entrada_nvl2->nro_pagina);
				free(pagina_vacia);
			}
			j++;
		}
		i++;
	}


}
bool esta_en_memoria(uint32_t proceso_id,uint32_t pagina){

	bool esta = false;

	Tabla_nvl1* tabla_1 = buscar_proceso_tabla1(proceso_id);

	t_list_iterator* i_list_aux = list_iterator_create(tabla_1->lista_aux);

	while(list_iterator_has_next(i_list_aux)){

		Remplazo_paginas* remplazo = list_iterator_next(i_list_aux);

		if(pagina == remplazo->pag_en_memoria){
			esta = true;
		}

	}
	list_iterator_destroy(i_list_aux);
	return esta;
}

uint32_t retorntar_nro_tabla_nvl2(uint32_t id_tabla_nivel_1, uint32_t entrada_tabla_nivel_1){

	Tabla_nvl1* tabla_nivel_1= (Tabla_nvl1*)list_get(tabla_de_paginas, id_tabla_nivel_1);
	Entrada_tabla_nvl1* tabla_nivel_2 = (Entrada_tabla_nvl1*)list_get(tabla_nivel_1->Entradas_tabla_nvl1, entrada_tabla_nivel_1);
	return tabla_nivel_2->nro_tabla_nvl2;
}

Tabla_nvl1* buscar_proceso_tabla1(uint32_t proceso_id){


	t_list_iterator* tabla = list_iterator_create(tabla_de_paginas);
	int i=0;

	while(list_iterator_has_next(tabla)){

		Tabla_nvl1* tabla_nvl1 = list_iterator_next(tabla);

		if(tabla_nvl1->proceso_id == proceso_id){
			break;
		}
		i++;
	}
	list_iterator_destroy(tabla);

	Tabla_nvl1* nodo =(Tabla_nvl1*) list_get(tabla_de_paginas,i);//busco y lo remuevo

	return nodo;
}

Tabla_nvl1* tabla_nivel_1_a_usar(uint32_t cant_tablas_nvl2){

	//recorrer tabla de paginas
	t_list_iterator* tabla_de_paginas_i = list_iterator_create(tabla_de_paginas);

	bool tabla_encontrada = false;
	Tabla_nvl1* tabla_nvl1;
	int index_tabla =0;

	while(list_iterator_has_next(tabla_de_paginas_i)){

		tabla_nvl1 = list_iterator_next(tabla_de_paginas_i);

		if((list_size(tabla_nvl1->Entradas_tabla_nvl1)+cant_tablas_nvl2)<=ENTRADAS_POR_TABLA){
			tabla_encontrada = true;
			break;
		}
		index_tabla++;
	}
	list_iterator_destroy(tabla_de_paginas_i);

	if(tabla_encontrada){
		return list_remove(tabla_de_paginas,index_tabla);//saci de la lista y retorno
	}else{
		return instanciar_tabla_nvl1(1);
	}
}

uint32_t es_direccion_valida(uint32_t proceso_id,uint32_t pagina,uint32_t offset){
	uint32_t es_valida = 0;
	Tabla_nvl1*  proceso = buscar_proceso_tabla1(proceso_id);

	if(proceso->tam_proceso >= (pagina*TAM_PAGINA+offset)){
		es_valida = 1;
	}


	return es_valida;
}

void swapear_proceso(uint32_t proceso_id){
	Tabla_nvl1* tabla_1 = buscar_proceso_tabla1(proceso_id);
	int i = 0;
	while(i< list_size(tabla_1->Entradas_tabla_nvl1)){
		Entrada_tabla_nvl1* entrada_1 = (Entrada_tabla_nvl1*)list_get(tabla_1->Entradas_tabla_nvl1,i);
		int j= 0;
		while(j<list_size(entrada_1->entradas_tabla_nvl2)){
			Entrada_tabla_nvl2* entrada_2 = (Entrada_tabla_nvl2*)list_get(entrada_1->entradas_tabla_nvl2,j);
			if(entrada_2->bit_presencia == 1 && entrada_2->bit_modificado == 1){

				log_info(log_tester,"La pagina a swapear es : %i en el marco: %i ",entrada_2->nro_pagina,entrada_2->nro_marco);
				void* pagina = leer_pagina_eu(entrada_2 ->nro_marco);
				escribir_en_swap(tabla_1->archivo_swap,tabla_1->tam_archivo,entrada_2->nro_pagina,pagina);
				//log_info(log_tester,"La pagina: %i fue mandada a swap",entrada_2->nro_pagina);
				free(pagina);
			}
			j++;
		}
		i++;
	}

}
void swapear_pagina(Tabla_nvl1* tabla_1,uint32_t nro_marco,uint32_t nro_pagina){

	void* pagina = leer_pagina_eu(nro_marco);

	escribir_en_swap(tabla_1->archivo_swap,tabla_1->tam_archivo,nro_pagina,pagina);

	free(pagina);
}

void borrar_archivo_de_memoria(uint32_t proceso_id){
	Tabla_nvl1* tabla_1 = buscar_proceso_tabla1(proceso_id);

	borrar_archivo(tabla_1->archivo_swap);
}

void* leer_pagina_eu(uint32_t marco){
	void*pagina_a_leer= malloc(TAM_PAGINA);
	uint32_t offset = marco*TAM_PAGINA;
	memcpy(pagina_a_leer,espacio_usuario+offset,TAM_PAGINA);

	return pagina_a_leer;

}
void escribir_pagina_eu(uint32_t dir_fisica, void* pagina){

	memcpy(espacio_usuario+dir_fisica,pagina,TAM_PAGINA);
}
void traer_pagina_a_eu(Tabla_nvl1* tabla1,uint32_t nro_pagina,uint32_t nro_marco){

	uint32_t dir_fisica =nro_marco*TAM_PAGINA;
	void*pagina = lectura_de_swap(tabla1->archivo_swap,nro_pagina,tabla1->tam_archivo);

	log_info(log_global,"nro_pagina: %i, nro_marco: %i",nro_pagina,nro_marco);

	escribir_pagina_eu(dir_fisica,pagina);
	free(pagina);

	//   64 -> 0 al 63 -> 64 al 127
	//	free(pagina);

}


