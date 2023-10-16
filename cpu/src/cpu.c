/*
 ============================================================================
 Name        : cpu.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "./server/server.h"

void inicializar_cpu();

void pepito();

int main(int argc, char **argv) {

	/* 1. Log */
	char*nombre_aux= string_duplicate(argv[1]);
	inicializar_loggers();
	inicializar_config(nombre_aux);
	inicializar_tlb();
	incializar_cola_para_algoritmo();

	/* 2. Configuración */

	inicializar_cpu();

	/* 3. Conexion */
	/*pcb proceso_0;
	pcb_global.id = 0;
	pcb_global.tamanio = 64*6;
	pcb_global.tabla_paginas = 0;
	obtener_direccion_fisica(6000);*/
	/* 4. Servidor */

	inicializar_variables();

//	escribir_en_memoria(50, 300);
	//printf("\n leer: %i \n",leer_de_memoria(50));
	pthread_t thread_server_dispatch;
	pthread_t thread_server_interrupt;

	if(pthread_create(&thread_server_dispatch, NULL, (void*)levantar_server_dispatch, NULL)) {
		pthread_mutex_lock(&sem_log_global);
		log_error(log_global, "Error al crear el thread para el server dispatch.");
		pthread_mutex_unlock(&sem_log_global);
	}

	if(pthread_create(&thread_server_interrupt, NULL, (void*)levantar_server_interrupt, NULL)) {
		pthread_mutex_lock(&sem_log_global);
		log_error(log_global, "Error al crear el thread para el server interrupt.");
		pthread_mutex_unlock(&sem_log_global);
	}

/*
	TAM_PAGINA = 1024; //TODO: Borrar
	ENTRADAS_TLB = 4;
	if(tlb_vacia()) {
		printf("La TLB esta vacia \n)");
	}

	printf ("La cantidad de entradas en la TLB es: %i \n", dictionary_size(tlb));

	printf("La direccion fisica para la DL 5420 es: %i \n\n", obtener_direccion_fisica(5420));
	printf ("La cantidad de entradas en la TLB es: %i \n", dictionary_size(tlb));
	printf("La direccion fisica para la DL 3172 es: %i \n\n", obtener_direccion_fisica(3172));
	printf ("La cantidad de entradas en la TLB es: %i \n", dictionary_size(tlb));
	printf("La direccion fisica para la DL 4097 es: %i \n\n", obtener_direccion_fisica(4097));
	printf ("La cantidad de entradas en la TLB es: %i \n", dictionary_size(tlb));
	printf("La direccion fisica para la DL 2068 es: %i \n\n", obtener_direccion_fisica(2068));
	printf ("La cantidad de entradas en la TLB es: %i \n", dictionary_size(tlb));
	printf("La direccion fisica para la DL 15 es: %i \n\n", obtener_direccion_fisica(15));
	printf ("La cantidad de entradas en la TLB es: %i \n", dictionary_size(tlb));
	printf("La direccion fisica para la DL 5138 es: %i \n\n", obtener_direccion_fisica(5138));
	printf ("La cantidad de entradas en la TLB es: %i \n", dictionary_size(tlb));
	printf("La direccion fisica para la DL 4140 es: %i \n\n", obtener_direccion_fisica(4140));
	printf ("La cantidad de entradas en la TLB es: %i \n", dictionary_size(tlb));
	printf("La direccion fisica para la DL 3388 es: %i \n\n", obtener_direccion_fisica(3388));
	printf ("La cantidad de entradas en la TLB es: %i \n", dictionary_size(tlb));
	printf("La direccion fisica para la DL 3133 es: %i \n\n", obtener_direccion_fisica(3133));
	printf ("La cantidad de entradas en la TLB es: %i \n", dictionary_size(tlb));
	printf("La direccion fisica para la DL 39 es: %i \n\n", obtener_direccion_fisica(39));
	printf ("La cantidad de entradas en la TLB es: %i \n", dictionary_size(tlb));*/

	pthread_join(thread_server_dispatch, NULL);
	pthread_join(thread_server_interrupt, NULL);

	/* 5. Liberar */

	destruir_tlb();
	destruir_cola_para_algoritmo();
	destruir_config(); // Tiene que ir despues de la destruccion de las colas para los algoritmos
	destruir_loggers();

	pthread_mutex_destroy(&sem_interrupcion);
	pthread_mutex_destroy(&sem_log_global);

	return 0;
}

void pepito() {
	printf("\n\nHola don pepito\n\n");
}

void inicializar_cpu(){
	int tam = sizeof(uint32_t);
	void* buffer = malloc(8);
	int offset = 0;
	int socket_memoria = conectar_con_memoria(IP_MEMORIA,PUERTO_MEMORIA);
	enviar_mensaje("",socket_memoria,INICIAR_CPU); // considerar uso de enviar_mensaje_inicial_memoria(); TODO agregue un 1 para que no rompa

	recv(socket_memoria, buffer, tam*2, MSG_WAITALL);

	memcpy(&ENTRADAS_POR_TABLA_MEMORIA,buffer+offset,tam);
	offset += tam;
	memcpy(&TAM_PAGINA,buffer+4,tam);
	log_debug(log_global, "Se recibio de memoria ENTRADAS_POR_TABLA: %i, TAM_PAGINA: %i \n", ENTRADAS_POR_TABLA_MEMORIA, TAM_PAGINA);

	//ENTRADAS_POR_TABLA_MEMORIA = 10; //TODO: Borrar
	//TAM_PAGINA = 1024; //TODO: Borrar

	free(buffer);
	liberar_conexion(socket_memoria);
	pthread_mutex_init(&sem_interrupcion, NULL);
	pthread_mutex_init(&sem_log_global, NULL);

}

void inicializar_config(char* nombre_config) {

	char*RUTA_CONFIG =malloc(strlen("../Configs/")+1);
	strcpy(RUTA_CONFIG,"../Configs/");
	string_append(&RUTA_CONFIG,nombre_config);
	config = config_create(RUTA_CONFIG);
	free(RUTA_CONFIG);

	ENTRADAS_TLB = (uint32_t) config_get_int_value(config,"ENTRADAS_TLB");

	char* reemplazo_tlb = config_get_string_value(config, "REEMPLAZO_TLB");
	REEMPLAZO_TLB = malloc(strlen(reemplazo_tlb)+1);
	strcpy(REEMPLAZO_TLB, reemplazo_tlb);

	RETARDO_NOOP = config_get_int_value(config,"RETARDO_NOOP");

	char* mem_ip = config_get_string_value(config, "IP_MEMORIA");
	IP_MEMORIA = malloc(strlen(mem_ip)+1);
	strcpy(IP_MEMORIA, mem_ip);


	PUERTO_MEMORIA = config_get_int_value(config,"PUERTO_MEMORIA");

	PUERTO_ESCUCHA_DISPATCH = config_get_int_value(config,"PUERTO_ESCUCHA_DISPATCH");
	PUERTO_ESCUCHA_INTERRUPT = config_get_int_value(config,"PUERTO_ESCUCHA_INTERRUPT");

	printf("El puerto dispatch es: %i \n", PUERTO_ESCUCHA_DISPATCH);
	printf("El puerto interrupt es: %i \n", PUERTO_ESCUCHA_INTERRUPT);

}

void destruir_config() {
	free(IP_MEMORIA);
	free(REEMPLAZO_TLB);

	config_destroy(config);
}

void inicializar_loggers() {
	remove("../cpu.log");
	remove("../log_test.log");
	remove("../log_usar.log");
	log_global = log_create("../cpu.log", "LOG-GLOBAL-CPU", 1, LOG_LEVEL_TRACE);
	log_test = log_create("../log_test.log", "Test", 1, LOG_LEVEL_TRACE);
	log_usando = log_create("../log_usar.log", "Test", 1, LOG_LEVEL_TRACE);

	pthread_mutex_init(&sem_log_global, NULL);
}

void destruir_loggers() {
	pthread_mutex_destroy(&sem_log_global);
	log_destroy(log_global);
	log_destroy(log_test);
}

void inicializar_tlb(){
	tlb = dictionary_create();
}

void destruir_tlb() {
	void destruir(uint32_t* marco){
		free(marco);
	}
	dictionary_destroy_and_destroy_elements(tlb,(void*)destruir);
}

void incializar_cola_para_algoritmo() {
	if(es_algoritmo("FIFO")) {
		cola_fifo = queue_create();
	}
	if(es_algoritmo("LRU")) {
		cola_lru = list_create();
	}
}

void destruir_cola_para_algoritmo() {
	if(es_algoritmo("FIFO")) {
		queue_destroy(cola_fifo);
	}
	if(es_algoritmo("LRU")) {
		list_destroy(cola_lru);
	}

}

