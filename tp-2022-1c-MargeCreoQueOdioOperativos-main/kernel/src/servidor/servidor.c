#include "servidor.h"

int levantar_servidor() {

	remove("log_servidor.log");
	log_servidor = log_create("log_servidor.log","LOG-SERVIDOR-KERNEL", 1, LOG_LEVEL_DEBUG);
	pthread_mutex_init(&sem_log_servidor, NULL);

	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = INADDR_ANY;
	direccionServidor.sin_port = htons(PUERTO_ESCUCHA);

	socket_servidor = socket(AF_INET, SOCK_STREAM , 0); // socket_servidor sería el File Descriptor que devuelve socket()

	log_info(log_servidor, "Servidor Kernel arriba!");

	int activado = 1;
	setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

	server_levantado = true;

	if(bind(socket_servidor, (void*) &direccionServidor, sizeof(direccionServidor)) != 0) {
		log_error(log_servidor, "Fallo el bind.");

		server_levantado = false;

		return 1;
	}

	while(server_levantado) {
		int cliente;

		listen(socket_servidor, 100);

		pthread_t thread_cliente;
		struct sockaddr_in direccion_cliente;
		unsigned tamanio_direccion = sizeof(struct sockaddr_in);

		cliente = accept(socket_servidor, (void*) &direccion_cliente, &tamanio_direccion);

		pthread_mutex_lock(&sem_log_servidor);
		log_info(log_servidor, "Se conecto un nuevo cliente.");
		pthread_mutex_unlock(&sem_log_servidor);

		pthread_create(&thread_cliente, NULL, (void*)atender_cliente, (void*)cliente);
		pthread_detach(thread_cliente);
	}

	log_info(log_servidor, "Servidor Kernel cerrado.");

	pthread_mutex_destroy(&sem_log_servidor);
	log_destroy(log_servidor);

	return 0;
}

bool patova(int32_t sock_cliente) {
	char* lectura = (char*)malloc(strlen("holo")+1);
	recv(sock_cliente, lectura, strlen("holo")+1, 0); // Si pasan cosas, revisar el flag
	uint32_t validacion =  strcmp(lectura, "holo");

	free(lectura);

	return validacion == 0;
}

void atender_cliente(int socket_cliente) {
	void* buffer;
	int size;

	if(patova(socket_cliente)) {

		int cod_op = recibir_operacion(socket_cliente);
		buffer = recibir_buffer(&size, socket_cliente);
		ejecutar_operacion(buffer, cod_op, socket_cliente);

		free(buffer);

	} else {
		close(socket_cliente);

		pthread_mutex_lock(&sem_log_servidor);
		log_error(log_servidor, "Me mandaron basura.");
		pthread_mutex_unlock(&sem_log_servidor);
	}

}

void ejecutar_operacion(void* buffer, int codigo_operacion, int socket_cliente) {
	uint32_t tamanio_proceso = 0;
	t_list* instrucciones;

	switch(codigo_operacion) {

	case 1002: // NUEVO_PROCESO

		pthread_mutex_lock(&sem_log_servidor);
		log_info(log_servidor, "Recibi un pedido de NUEVO_PROCESO por parte de un cliente.");
		pthread_mutex_unlock(&sem_log_servidor);

		tamanio_proceso = get_tamanio_proceso(buffer);
		instrucciones = list_create();
		get_instrucciones(buffer, instrucciones);

		crear_proceso(instrucciones, tamanio_proceso, socket_cliente);
		sem_post(sem_nuevo_proceso);

		void instruction_destroy(t_instruccion* instruccion) {

			free(instruccion);
		}

		list_destroy_and_destroy_elements(instrucciones, (void*)instruction_destroy);

//		pthread_mutex_lock(&sem_log_servidor);
//		log_info(log_servidor, "Se atendio el pedido de paquete NUEVO_PROCESO correctamente.");
//		pthread_mutex_unlock(&sem_log_servidor);

		break;

	default:

		pthread_mutex_lock(&sem_log_servidor);
		log_error(log_servidor, "El codigo de operacion recibido es incorrecto!");
		pthread_mutex_unlock(&sem_log_servidor);

		break;
	}

}

void cerrar_servidor() {
	close(socket_servidor);
	server_levantado = false;
}
