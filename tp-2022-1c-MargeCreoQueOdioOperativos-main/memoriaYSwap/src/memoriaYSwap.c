/*
 ============================================================================
 Name        : memoriaYSwap.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "memoriaYSwap.h"

int main(int argc,char** argv) {

	char*nombre_aux= string_duplicate(argv[1]);
	inicializar_config(nombre_aux);
	iniciar_tabla_marcos_disponibles();
	espacio_usuario = malloc(TAM_MEMORIA);


	pthread_t thread_server;

	pthread_create(&thread_server, NULL, (void*)levantar_server_singlethread, NULL);

	pthread_join(thread_server, 0);



	liberar_memoria();

	return EXIT_SUCCESS;
}


