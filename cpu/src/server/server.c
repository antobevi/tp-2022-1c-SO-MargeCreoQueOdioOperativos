#include "server.h"

int levantar_servidor_en_puerto(int puerto) {
//	remove("../log_servidor.log");
//	log_servidor = log_create("../log_servidor.log","LOG-SERVIDOR-CPU", 1, LOG_LEVEL_DEBUG);
//
//	struct sockaddr_in direccionServidor;
//	direccionServidor.sin_family = AF_INET;
//	direccionServidor.sin_addr.s_addr = INADDR_ANY;
//	direccionServidor.sin_port = htons(puerto);
//
//	socket_servidor = socket(AF_INET, SOCK_STREAM , 0);
//	log_info(log_servidor, "Servidor CPU arriba!");
//
//	int activado = 1;
//	setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));
//
//	server_levantado = true;
//
//	if(bind(socket_servidor, (void*) &direccionServidor, sizeof(direccionServidor)) != 0) {
//		log_info(log_servidor, "Fallo el bind.");
//		server_levantado = false;
//
//		return 1;
//	}
//
//	while(server_levantado) {
//
//		listen(socket_servidor, 100);
//
//		int cliente;
//
//		struct sockaddr_in direccion_cliente;
//		unsigned tamanio_direccion = sizeof(struct sockaddr_in);
//
//		cliente = accept(socket_servidor, (void*) &direccion_cliente, &tamanio_direccion);
//		log_info(log_servidor, "Recibi una conexion de un cliente.");
//
//		atender_cliente(cliente);
//	}
//
//	log_info(log_servidor, "Servidor CPU cerrado.");
//	log_destroy(log_servidor);
//
	return 0;
}

bool validar(int32_t sock_cliente) {
	char* lectura = (char*)malloc(strlen("holo")+1);
    recv(sock_cliente, lectura, strlen("holo")+1, 0); // Si pasan cosas, revisar el flag
	uint32_t validacion =  strcmp(lectura, "holo");

	free(lectura);

	return validacion == 0;
}

int recibir_operacion(int socket_cliente) {
	int cod_op;

	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
	else {
		close(socket_cliente);
		return -1;
	}
}

void* recibir_buffer(int* size, int socket_cliente) {
	void* buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

void atender_cliente(int socket_cliente) {
	void* buffer;
	int size;

	if(validar(socket_cliente)) {
		int cod_op = recibir_operacion(socket_cliente);
		buffer = recibir_buffer(&size, socket_cliente);
		//en el buffer me viene toodo lo que kernel me manda
		ejecutar_operacion(buffer, cod_op, socket_cliente);
		free(buffer);

	} else {
		close(socket_cliente);
		log_info(log_servidor, "Me mandaron basura, lo ignoro.");
	}
}

int levantar_server_dispatch() {

	remove("../log_servidor.log");
	log_servidor = log_create("../log_servidor.log","LOG-SERVIDOR-CPU", 1, LOG_LEVEL_DEBUG);

	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = INADDR_ANY;
	direccionServidor.sin_port = htons(PUERTO_ESCUCHA_DISPATCH);

	socket_servidor_dispatch = socket(AF_INET, SOCK_STREAM , 0);
	log_info(log_servidor, "Servidor CPU DISPATCH arriba!");

	int activado = 1;
	setsockopt(socket_servidor_dispatch, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

	server_levantado_dispatch = true;

	if(bind(socket_servidor_dispatch, (void*) &direccionServidor, sizeof(direccionServidor)) != 0) {
		log_info(log_servidor, "Fallo el bind en el servidor dispatch.");
		server_levantado_dispatch = false;

		return 1;
	}

	while(server_levantado_dispatch) {
		int cliente;

		listen(socket_servidor_dispatch, 100);

		struct sockaddr_in direccion_cliente;
		unsigned tamanio_direccion = sizeof(struct sockaddr_in);

		cliente = accept(socket_servidor_dispatch, (void*)&direccion_cliente, &tamanio_direccion);
		log_info(log_servidor, "Recibi una conexion de un cliente.");

		atender_cliente(cliente);
	}

	log_info(log_servidor, "Servidor CPU cerrado.");
	log_destroy(log_servidor);

	return 0;

}

int levantar_server_interrupt() {

//	remove("../log_servidor.log"); TODO: Corregir los logs de ambos servidores
	//log_servidor = log_create("../log_servidor.log","LOG-SERVIDOR-CPU", 1, LOG_LEVEL_DEBUG);

	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = INADDR_ANY;
	direccionServidor.sin_port = htons(PUERTO_ESCUCHA_INTERRUPT);

	socket_servidor_interrupt = socket(AF_INET, SOCK_STREAM , 0);
	//log_info(log_servidor, "Servidor CPU INTERRUPT arriba!");

	printf("\n\nSocket escucha servidor INTERRUPT: %i\n\n", socket_servidor_interrupt);

	int activado = 1;
	setsockopt(socket_servidor_interrupt, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

	server_levantado_interrupt = true;

	if(bind(socket_servidor_interrupt, (void*) &direccionServidor, sizeof(direccionServidor)) != 0) {
		//log_info(log_servidor, "Fallo el bind en el servidor interrupt.");
		server_levantado_interrupt = false;

		return 1;
	}

	while(server_levantado_interrupt) {

		listen(socket_servidor_interrupt, 100);

		int cliente;

		struct sockaddr_in direccion_cliente;
		unsigned tamanio_direccion = sizeof(struct sockaddr_in);

		cliente = accept(socket_servidor_interrupt, (void*) &direccion_cliente, &tamanio_direccion);
		//log_info(log_servidor, "Recibi una conexion de un cliente.");

		atender_cliente(cliente);
	}

	//log_info(log_servidor, "Servidor CPU cerrado.");
	//log_destroy(log_servidor);

	return 0;

}

void iniciar_cuenta_tiempo_ejecucion() {
	gettimeofday(&inicio_ejecucion, NULL);
}

void ejecutar_operacion(void* buffer, int cod_op, int socket_cliente){
	switch(cod_op) {
	case 1005: // PROCESO_A_EJECUTAR

		conexion_dispatch = socket_cliente;

		//TODO: deserializar el pcb que me manda kernel, y guardarlo en pcb_global
		//ejecutar iclo de intruccion con el while

		recibir_proceso_a_ejecutar_y_deserializar(buffer);

		log_debug(log_servidor, "El proceso con pid %i llego de Kernel para ser ejecutado \n", pcb_global.id);

		hay_proceso_para_ejecutar = 1;

		// Se inicia la cuenta de tiempo de ráfagas de CPU
		iniciar_cuenta_tiempo_ejecucion();


		while(hay_proceso_para_ejecutar) {
			ejecutar_ciclo_instruccion();
		}

		list_destroy_and_destroy_elements(pcb_global.instrucciones, (void*) destruir_instruccion);

		break;
	case 1009: // INTERRUPCION

		log_debug(log_servidor, "Llego una interrupcion de Kernel. El proceso sera desalojado \n");

		pthread_mutex_lock(&sem_interrupcion);
		interrupcion_pendiente = 1;
		pthread_mutex_unlock(&sem_interrupcion);

		break;
	}
}

void destruir_instruccion(t_instruccion* instruccion) {

    free(instruccion);
}
