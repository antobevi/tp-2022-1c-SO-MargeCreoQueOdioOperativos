#include "servidor.h"

uint32_t levantar_server_singlethread() {



	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family= AF_INET;
	direccionServidor.sin_addr.s_addr= INADDR_ANY;
	direccionServidor.sin_port=htons(PUERTO);

	uint32_t servidor = socket(AF_INET, SOCK_STREAM , 0);
	//loggear_mensaje("Server Broker arriba", 0);
	printf("Server memoria arriba\n");

	int activado = 1;
	setsockopt(servidor, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

	if( bind(servidor, (void*) &direccionServidor, sizeof(direccionServidor)) != 0){
		//loggear_mensaje("Fallo del bind", 0);
		server_levantado=false;
		return 1;
	}

	server_levantado=true;


	//	sem_init(&sem_conexion, 0, 1);
	//sem_init(&sem_conectar_con_servidor, 0, 1);

	while(server_levantado){
		int cliente;
		listen(servidor, 100);
		//pthread_t hilo;
		struct sockaddr_in direccionCliente;
		unsigned tamanioDireccion= sizeof(struct sockaddr_in);
		cliente = accept(servidor, (void*) &direccionCliente, &tamanioDireccion);

		atender_cliente(cliente);
		//sem_wait(&sem_conexion);
		//pthread_create(&hilo, NULL, (void*)atender_cliente, cliente);
		//pthread_detach(hilo);
	}

	//loggear_mensaje("Server cerrado", 0);
	printf("Server cerrado\n");
	//log_destroy(logger);
	//sem_destroy(&mutex_log);

	return 0;
}


void* recibir_paquete(uint32_t* alocador,uint32_t socket) {
	void* buffer;

	recv(socket,alocador,sizeof(uint32_t),MSG_WAITALL);
	buffer=malloc(*alocador);
	recv(socket,buffer,*alocador,MSG_WAITALL);

	return buffer;
}

bool patova(int32_t sock_cliente) {
	char* lectura = (char*)malloc(strlen("holo")+1);
	recv(sock_cliente, lectura, strlen("holo")+1, 0); //Si pasan cosas, revisar el flag
	uint32_t validacion =  strcmp(lectura, "holo");

	free(lectura);

	return validacion == 0;
}

op_code conseguir_codigo_operacion(int32_t socket) {
	op_code codigo_operacion;

	recv(socket, &codigo_operacion, sizeof(op_code), MSG_WAITALL);

	return codigo_operacion;
}

// TODO: Cambiar por funciones correspondientes

void atender_cliente(int32_t sock_cliente) {
	void* buffer;
	uint32_t alocador;
	//sem_wait(&sem_conexion);

	if(patova(sock_cliente)) {
		//loggear_mensaje("Se conecto un cliente. Hilo de atencion disparado", 1);

		op_code codigo_operacion = conseguir_codigo_operacion(sock_cliente);
		buffer=recibir_paquete(&alocador,sock_cliente);

		ejecutar_operacion(buffer,codigo_operacion,sock_cliente);

		//TODO: Deberiamos revisar el buffer antes de hacer nada a ver si todavia esta vacio.

		free(buffer);

		//		printf("Se atendio el pedido correctamente. %i \n",codigo_operacion);
	} else {
		close(sock_cliente);
		//	printf("Me mandaron basura, lo ignoro.\n");
	}

	//sem_post(&sem_conexion);

}

void ejecutar_operacion(void* buffer, op_code codigo_operacion,int32_t sock_cliente){

	uint32_t proceso_id=0;
	uint32_t tamanio = 0;
	uint32_t pagina=0;
	uint32_t entrada_nvl1;
	uint32_t entrada_nvl2;
	uint32_t id_tabla_nivel_1;
	uint32_t id_tabla_nivel_2;

	uint32_t respuesta = 1;
	uint32_t offset;


	void* respuesta_peticion = malloc(sizeof(uint32_t));

	void* envio_nro_tabla_nvl1;
	void* recibido_id_tabla_nivel_1_y_entrada_nivel_1;
	void* envio_nro_tabla_nvl2;
	void* recibido_id_tabla_nivel_2_y_entrada_nivel_2;
	void* envio_marco;

	void* valores_cpu;
	uint32_t nro_marco = 4;
	void*lectura;

	//uint32_t pagina_reemplazada = 0;

	uint32_t dir_fisica;
	uint32_t valor;
	switch(codigo_operacion) {

	case INICIAR_PROCESO:
		sleep(RETARDO_MEMORIA);

		obtener_id_y_tamanio(buffer,&proceso_id,&tamanio);

		log_debug(log_global,"\n Llego el proceso:%i  ",proceso_id);
		//log_debug(log_tester,"\n  de tamaño: %i",tamanio);

		id_tabla_nivel_1 = iniciar_proceso_en_memoria(proceso_id,tamanio);
		envio_nro_tabla_nvl1 = malloc(sizeof(uint32_t));

		memcpy(envio_nro_tabla_nvl1,&id_tabla_nivel_1,4);
		log_info(log_global,"Nro de tabla enviado a Kernel ");
		send(sock_cliente,envio_nro_tabla_nvl1,sizeof(uint32_t),0);

		free(envio_nro_tabla_nvl1);


		log_debug(log_global,"\n--- Se atendio el pedido correctamente de INICIAR  PROCESO:Kernel ---\n");
		break;
	case SUSPENSION_PROCESO:
		sleep(RETARDO_MEMORIA);

		memcpy(&proceso_id,buffer,sizeof(uint32_t));

		log_debug(log_global,"El proceso %i ha sido enviado para suspender",proceso_id);


		suspender_proceso_en_memoria(proceso_id);
		respuesta = 1011;

		memcpy(respuesta_peticion,&respuesta,sizeof(uint32_t));
		send(sock_cliente,respuesta_peticion,sizeof(uint32_t),0);

		log_debug(log_global,"El proceso %i ha sido suspendido",proceso_id);


		log_debug(servidor,"\n--- Se atendio el pedido correctamente de SUSPENSION PROCESO:Kernel ---\n");

		break;

	case PROCESO_TERMINADO:// Finalizar proceso
		sleep(RETARDO_MEMORIA);

		memcpy(&proceso_id,buffer,sizeof(uint32_t));
		log_debug(log_global,"El proceso %i ha sido enviado para finalizar",proceso_id);

		finalizar_proceso(proceso_id);


		respuesta = 1013; // ESTRUCTURAS_LIBERADAS
		memcpy(respuesta_peticion,&respuesta,sizeof(uint32_t));
		send(sock_cliente,respuesta_peticion,sizeof(uint32_t),0);
		log_info(log_global,"Respuesta de finalizado de proceso enviada");

		log_debug(servidor,"\n--- Se atendio el pedido correctamente de FINALIZAR PROCESO:Kernel ---\n");
		break;


	case HANDSHAKE://HANDSHAKE
		sleep(RETARDO_MEMORIA);

		log_debug(log_global,"Handshake con CPU \n");
		valores_cpu= inicializar_cpu();
		send(sock_cliente,valores_cpu,sizeof(int)*2,0);
		log_debug(log_global,"Valores enviados a CPU\n");
		free(valores_cpu);

		log_debug(servidor,"\n--- Se atendio el pedido correctamente de HANDSHAKE:CPU---\n");
		break;
	case PEDIDO_MARCO://PEDIDO DE MARCO

		memcpy(&proceso_id,buffer,sizeof(uint32_t));
		memcpy(&pagina,buffer+sizeof(uint32_t),sizeof(uint32_t));
		memcpy(&offset,buffer+sizeof(uint32_t)*2,sizeof(uint32_t));

		log_debug(log_global,"\n El proceso %i quire un numero de marco para la pagina %i \n ",proceso_id,pagina);
		//log_debug(log_tester,"Offset %i",offset);

		respuesta = es_direccion_valida(proceso_id,pagina,offset);
		//log_info(log_global,"\n La direccion es valida %i \n",respuesta);

		memcpy(respuesta_peticion,&respuesta,sizeof(uint32_t));

		send(sock_cliente,respuesta_peticion,sizeof(uint32_t),0);
		log_debug(log_global,"\n--- Respuesta de direccion valida enviada a memoria---\n");

		if(respuesta){

			//----------PRIMER ACCESO----------
			recibido_id_tabla_nivel_1_y_entrada_nivel_1 = malloc(sizeof(uint32_t)*2);
			recv(sock_cliente, recibido_id_tabla_nivel_1_y_entrada_nivel_1, sizeof(uint32_t)*2,MSG_WAITALL);
			sleep(RETARDO_MEMORIA);
			memcpy(&id_tabla_nivel_1, recibido_id_tabla_nivel_1_y_entrada_nivel_1,sizeof(uint32_t));
			memcpy(&entrada_nvl1, recibido_id_tabla_nivel_1_y_entrada_nivel_1+sizeof(uint32_t),sizeof(uint32_t));
			free(recibido_id_tabla_nivel_1_y_entrada_nivel_1);

			log_info(log_global,"CPU envio la tabla de nivel 1: %i y la entrada: %i ", id_tabla_nivel_1, entrada_nvl1);

			//Reservo espacio para retornar el valor de la tabla
			envio_nro_tabla_nvl2 = malloc(sizeof(uint32_t));
			id_tabla_nivel_2 = retorntar_nro_tabla_nvl2(id_tabla_nivel_1,entrada_nvl1);
			memcpy(envio_nro_tabla_nvl2,&id_tabla_nivel_2,sizeof(uint32_t));
			send(sock_cliente,envio_nro_tabla_nvl2,sizeof(uint32_t),0);
			log_info(log_global,"Nro de tabla de nivel 2: %i, y entrada tabla nivel 1: %i \n", id_tabla_nivel_2, entrada_nvl1);
			//----------PRIMER ACCESO----------



			//----------SEGUNDO ACCESO----------
			//Recivo la entrada de segundo nivel
			recibido_id_tabla_nivel_2_y_entrada_nivel_2 = malloc(sizeof(uint32_t)*2);
			recv(sock_cliente, recibido_id_tabla_nivel_2_y_entrada_nivel_2, sizeof(uint32_t)*2, MSG_WAITALL);
			sleep(RETARDO_MEMORIA);
			memcpy(&id_tabla_nivel_2, recibido_id_tabla_nivel_2_y_entrada_nivel_2,sizeof(uint32_t));
			memcpy(&entrada_nvl2, recibido_id_tabla_nivel_2_y_entrada_nivel_2+sizeof(uint32_t),sizeof(uint32_t));
			log_info(log_global,"Recibido: id de tabla de nivel 2: %i, entrada de nivel 2 : %i", id_tabla_nivel_2, entrada_nvl2);

			//Buscar marco con el numero de pagina
			envio_marco = malloc(sizeof(uint32_t)*3);//marco 0/1 pagina_remplazada
			hubo_remplazo = REEMPLAZO;
			//pagina_reemplazada;

			nro_marco = devolver_marco(id_tabla_nivel_1, entrada_nvl1, entrada_nvl2);

			log_debug(log_global,"Nro de marco: %i",nro_marco);
			if(!hubo_remplazo){
				log_info(log_global,"Hubo remplazo: %i , Pagina remplazada: %i ",hubo_remplazo,pagina_reemplazada);
			}

			memcpy(envio_marco,&nro_marco,sizeof(uint32_t));
			memcpy(envio_marco+sizeof(uint32_t),&hubo_remplazo,sizeof(uint32_t));
			memcpy(envio_marco+sizeof(uint32_t)+sizeof(uint32_t),&pagina_reemplazada,sizeof(uint32_t));

			send(sock_cliente,envio_marco,sizeof(uint32_t)*3,0);
			log_info(log_global,"Marco enviado a CPU\n");
			//----------SEGUNDO ACCESO----------


			free(envio_nro_tabla_nvl2);
			free(recibido_id_tabla_nivel_2_y_entrada_nivel_2);
			free(envio_marco);
		}


		log_debug(log_global,"\n--- Se atendio el pedido correctamente de PEDIDO MARCO:CPU---\n");
		break;


	case LECTURA_MEMORIA://lectura
		sleep(RETARDO_MEMORIA);

		memcpy(&dir_fisica,buffer,sizeof(uint32_t));
		memcpy(&proceso_id,buffer+sizeof(uint32_t),sizeof(uint32_t));

		lectura = malloc(sizeof(uint32_t));
		valor = leer_en_eu(dir_fisica);

		memcpy(lectura,&valor,sizeof(uint32_t));
		log_debug(log_global,"El proceso:%i leyo el valor:%i en la direccion %i ",proceso_id,valor,dir_fisica);

		send(sock_cliente,lectura,sizeof(uint32_t),0);
		free(lectura);

		log_debug(log_global,"\n--- Se atendio el pedido correctamente de PEDIDO LECTURA:CPU---\n");

		break;
	case ESCRITURA_MEMORIA://Escritura
		sleep(RETARDO_MEMORIA);

		memcpy(&dir_fisica,buffer,sizeof(uint32_t));
		memcpy(&valor,buffer+sizeof(uint32_t),sizeof(uint32_t));
		memcpy(&proceso_id,buffer+sizeof(uint32_t)*2,sizeof(uint32_t));
		log_debug(log_global,"El proceso %i escribio el valor %i en la direccion %i",proceso_id,valor,dir_fisica);

		cambiar_bit_m(proceso_id,dir_fisica);
		escribir_en_eu(dir_fisica,valor);


		memcpy(respuesta_peticion,&respuesta,sizeof(uint32_t));
		send(sock_cliente,respuesta_peticion,sizeof(uint32_t),0);

		log_debug(log_global,"\n--- Se atendio el pedido correctamente de PEDIDO ESCRITURA:CPU---\n");
		break;


	}
		//leer_tabla();
	free(respuesta_peticion);
}


void obtener_id_y_tamanio(void* buffer,uint32_t* id,uint32_t* tamanio){

	int desplazamiento = 0;

	memcpy(id, buffer + desplazamiento, sizeof(uint32_t));

	desplazamiento += sizeof(int);


	memcpy(tamanio, buffer + desplazamiento, sizeof(uint32_t));
}

void* inicializar_cpu(){
	int offset = 0;
	int tam = sizeof(int);
	void*buffer = malloc(tam*2);

	memcpy(buffer+offset,&ENTRADAS_POR_TABLA,tam);

	offset += tam;

	memcpy(buffer+offset,&TAM_PAGINA,tam);

	return buffer;
}
