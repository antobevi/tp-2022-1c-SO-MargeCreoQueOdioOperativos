/*
 * cicloInstruccion.c
 *
 *  Created on: 31 may. 2022
 *      Author: utnso
 */

#include "cicloInstruccion.h"


void inicializar_variables() {
	buscar_operandos = 0;
	interrupcion_pendiente = 0;
}

void ejecutar_ciclo_instruccion() {

	//Cuando llegue un proceso nuevo de kernel, el pcb se almacenara en la variable global pcb

	t_instruccion instruccion = fetch();

	decode(instruccion.identificador);

	fetch_operands(instruccion);

	execute(instruccion);

	check_interrupt();
}

t_instruccion fetch() {
	uint32_t program_counter = pcb_global.program_counter;
	t_instruccion instruccion = *((t_instruccion*) list_get(pcb_global.instrucciones, (int) program_counter));
	pcb_global.program_counter++;

	return instruccion;
}

void decode(char* identificador) {
	if(string_equals_ignore_case("COPY", identificador)) {
		buscar_operandos = 1;
	}
	else{
		buscar_operandos = 0;
	}
}

void fetch_operands(t_instruccion instruccion) {
	if(buscar_operandos) {
		if(string_equals_ignore_case("COPY", instruccion.identificador)) {
			uint32_t direccion_logica = instruccion.parametro_2;
			uint32_t direccion_fisica = obtener_direccion_fisica(direccion_logica);
			operando = buscar_operando_en_memoria(direccion_fisica);
		}
	}
}

void execute(t_instruccion instruccion) {
	if(string_equals_ignore_case("NO_OP", instruccion.identificador)) {

		//sleep(RETARDO_NOOP / 1000);
		usleep(RETARDO_NOOP * 1000);

		log_info(log_global, "Se ejecuta la operacion NO_OP con un retardo de %i milisegundos \n", RETARDO_NOOP);
	}

	if(string_equals_ignore_case("I/O", instruccion.identificador)) {
		log_info(log_global, "Se ejecuta una operacion de i/o, el PCB se devolvera a Kernel \n");
		uint32_t tiempo_bloqueo = instruccion.parametro_1;

		finalizar_cuenta_tiempo_ejecucion();
		calcular_tiempo_ejecucion();

		enviar_syscall_io_bloqueante_a_kernel(tiempo_bloqueo); // Se debe enviar además tiempo_ejecucion
		hay_proceso_para_ejecutar = 0;

	}

	if(string_equals_ignore_case("READ", instruccion.identificador)) {
		log_info(log_global, "Se ejecuta una operacion de lectura. Se realizara la traduccion de DL a DF \n");
		uint32_t direccion_logica = instruccion.parametro_1;
		uint32_t direccion_fisica = obtener_direccion_fisica(direccion_logica);
		if(resultado_validacion) {
			uint32_t valor_leido = leer_de_memoria(direccion_fisica);
			printf("\n \n \n");
			log_trace(log_usando,"El valor leido de memoria es: %i del proceso:%i en la dir fisica %i", valor_leido,pcb_global.id,direccion_fisica);
			printf("\n \n \n");
		}
		else {
			log_error(log_global, "No se pudo realizar la operacion de lectura a memoria. Direccion invalida");
		}
	}

	if(string_equals_ignore_case("WRITE", instruccion.identificador)) {
		log_info(log_global, "Se ejecuta una operacion de escritura. Se realizara la traduccion de DL a un \n");
		uint32_t direccion_logica = instruccion.parametro_1;
		uint32_t direccion_fisica = obtener_direccion_fisica(direccion_logica);
		if(resultado_validacion) {
			uint32_t valor = instruccion.parametro_2;
			escribir_en_memoria(direccion_fisica, valor);
		}
		else {
			log_error(log_global, "No se pudo realizar la operacion de escritura en emeoria. Direccion invalida");
		}
	}

	if(string_equals_ignore_case("COPY", instruccion.identificador)) {
		log_info(log_global, "Se ejecuta la instruccion COPY. Se hara una traduccion de direcciones y se escribira en memoria \n");
		uint32_t direccion_logica_destino = instruccion.parametro_1;
		uint32_t direccion_fisica_destino = obtener_direccion_fisica(direccion_logica_destino);
		if(resultado_validacion) {
			escribir_en_memoria(direccion_fisica_destino, operando);
		}
		else {
			log_error(log_global, "No se pudo realizar la operacion de copia en memoria. Direccion invalida");
		}
	}

	if(string_equals_ignore_case("EXIT", instruccion.identificador)) {
		log_info(log_global, "Se ejecuta la instruccion EXIT. El proceso sera devuelto a kernel \n");
		finalizar_cuenta_tiempo_ejecucion();
		calcular_tiempo_ejecucion();

		enviar_syscall_finalizacion_a_kernel(); // Se debe enviar además tiempo_ejecucion
		hay_proceso_para_ejecutar = 0;
	}
}

void check_interrupt() {
	if(hay_proceso_para_ejecutar && (!proxima_instruccion("I/O"))) {

		pthread_mutex_lock(&sem_interrupcion);
		if(interrupcion_pendiente) {
			printf("\n\nHAY UNA INTERRUPCION PENDIENTE DE ATENDER\n\n");
			desalojar_proceso();
		}
		pthread_mutex_unlock(&sem_interrupcion);

	}

	//Aca va a ser necesario un mutex
	pthread_mutex_lock(&sem_interrupcion);
	interrupcion_pendiente = 0;
	pthread_mutex_unlock(&sem_interrupcion);
}

// TODO: Posible solucion a las interrupciones antes de una io
bool proxima_instruccion(char* identificador) {
	bool retorno = false;

	if(pcb_global.program_counter < list_size(pcb_global.instrucciones) - 1) { // Si hay proxima instruccion ...
		t_instruccion* proxima_instruccion = (t_instruccion*)list_get(pcb_global.instrucciones, pcb_global.program_counter);

		if(string_equals_ignore_case(proxima_instruccion->identificador, identificador)) {
			retorno = true;
		} else {
			retorno = false;
		}

	} else { // No hay proxima instruccion, podria ser EXIT
		retorno = false;
	}

	return retorno;

}

//----------TERCER ACCESO----------
uint32_t leer_de_memoria(uint32_t direccion_fisica) {
	int tam = sizeof(uint32_t);
	uint32_t elemento_leido;
	void* buffer = malloc(tam);

	int socket_memoria = conectar_con_memoria(IP_MEMORIA,PUERTO_MEMORIA);
	log_warning(log_test,"\n leer paquete Socket: %i", socket_memoria);
	t_paquete* paquete = armar_paquete_lectura(direccion_fisica, pcb_global.id);
	enviar_paquete(paquete,socket_memoria);
	eliminar_paquete(paquete);

	recv(socket_memoria, buffer, tam, MSG_WAITALL);
	memcpy(&elemento_leido,buffer,sizeof(uint32_t));
	//log_debug(log_global, "Se leyo eun valor de memoria \n");
	free(buffer);
	liberar_conexion(socket_memoria);
	return elemento_leido;
}

uint32_t buscar_operando_en_memoria(uint32_t direccion_fisica) {
	operando = leer_de_memoria(direccion_fisica);
	return operando;
}

void enviar_syscall_io_bloqueante_a_kernel(uint32_t tiempo_bloqueo) {
	t_paquete* paquete_syscall_io_bloqueante = armar_paquete_syscall_io_bloqueante(pcb_global.id, tiempo_bloqueo, tiempo_ejecucion, pcb_global.program_counter);
	enviar_paquete(paquete_syscall_io_bloqueante, conexion_dispatch);
	eliminar_paquete(paquete_syscall_io_bloqueante);
	hay_proceso_para_ejecutar = 0;
	vaciar_tlb();
	//log_debug(log_global, "El proceso fue devuelto a kernel como consecuencia de una i/o \n");
}

void desalojar_proceso() {
	t_paquete* paquete_proceso_desalojado = armar_paquete_proceso_desalojado(pcb_global.id, tiempo_ejecucion, pcb_global.program_counter);
	enviar_paquete(paquete_proceso_desalojado, conexion_dispatch);
	eliminar_paquete(paquete_proceso_desalojado);
	hay_proceso_para_ejecutar = 0;
	vaciar_tlb();
	//log_debug(log_global, "El proceso fue devuelto a kernel como consecuencia de una interrupcion \n");
}

//----------TERCER ACCESO----------
void escribir_en_memoria(uint32_t direccion_fisica, uint32_t valor) {
	uint32_t* confirmacion = malloc(sizeof(uint32_t));

	int socket_memoria = conectar_con_memoria(IP_MEMORIA,PUERTO_MEMORIA);
	printf("\n Socket: %i", direccion_fisica);
	t_paquete* paquete = armar_paquete_escritura(direccion_fisica, valor, pcb_global.id);
	enviar_paquete(paquete,socket_memoria);
	eliminar_paquete(paquete);
	recv(socket_memoria, confirmacion, sizeof(uint32_t), MSG_WAITALL);
	log_trace(log_usando,"EL PROCESO %i Escribio:%i en la DIR FISICA: %i",pcb_global.id,valor,direccion_fisica);

	liberar_conexion(socket_memoria);
	free(confirmacion);
}

void enviar_syscall_finalizacion_a_kernel() {
	t_paquete* paquete_finalizacion_proceso = armar_paquete_finalizacion_proceso(pcb_global.id);
	enviar_paquete(paquete_finalizacion_proceso, conexion_dispatch);
	eliminar_paquete(paquete_finalizacion_proceso);
	vaciar_tlb();
	log_debug(log_global, "El proceso fue devuelto a kernel porque finalizo \n");
}

uint32_t obtener_direccion_fisica(uint32_t direccion_logica) {
	log_trace(log_test, "Se va a realizar una traduccion de DL a DF del proceso: %i  \n",pcb_global.id);

	uint32_t numero_pagina = (direccion_logica / TAM_PAGINA);// - (direccion_logica % TAM_PAGINA);
	uint32_t tabla_nivel_1 = pcb_global.tabla_paginas;
	uint32_t entrada_tabla_1er_nivel = (numero_pagina / ENTRADAS_POR_TABLA_MEMORIA);
	uint32_t entrada_tabla_2do_nivel =  numero_pagina % (ENTRADAS_POR_TABLA_MEMORIA);
	uint32_t desplazamiento = direccion_logica - (numero_pagina * TAM_PAGINA);
	//printf("El numero de pagina es: %i \n", numero_pagina);
	//printf("El offset es: %i \n", desplazamiento);
	uint32_t direccion_fisica = 0;
	uint32_t marco;

	if(pagina_esta_en_tlb(numero_pagina)) {
		marco = obtener_frame_de_tlb(numero_pagina);
		printf("El frame obtenido de la TLB es: %i \n", marco);
		direccion_fisica = marco * TAM_PAGINA + desplazamiento;
		actualizar_cola_de_algoritmo_por_referencia_a_una_pagina_en_tlb(numero_pagina);
		log_warning(log_test, "La pagina referenciada %i fue encontrada en la tlb  \n", numero_pagina);
	}
	else {
		uint32_t* hubo_reemplazo_en_memoria = malloc(sizeof(uint32_t));
		uint32_t* pagina_reemplazada_en_memoria = malloc(sizeof(uint32_t));
		uint32_t pagina_victima;
		//Acceder a memoria para obtener frame

		int socket_memoria = conectar_con_memoria(IP_MEMORIA,PUERTO_MEMORIA);

		resultado_validacion = validar_direccion_fisica(socket_memoria, pcb_global.id, numero_pagina,desplazamiento);

		if(resultado_validacion) {

			marco = obtener_frame_de_tabla_de_nivel_2(socket_memoria, tabla_nivel_1, entrada_tabla_1er_nivel, entrada_tabla_2do_nivel, hubo_reemplazo_en_memoria, pagina_reemplazada_en_memoria);
			//marco = 5;// TODO: borrar cuando funcione la conexion con memoria

			//Si hay posibles victimas, elegir una
			log_error(log_test," \n EL REMPLAZO ES  %i \n",*hubo_reemplazo_en_memoria);
			if(*hubo_reemplazo_en_memoria) {
				pagina_victima = *pagina_reemplazada_en_memoria;
				quitar_entrada_tlb(pagina_victima);
				actualizar_cola_de_algoritmo_para_dejar_tlb_consistente_por_reemplazo_hecho_en_memoria(pagina_victima, numero_pagina);
				log_trace(log_test, "Hubo reempazo de pagina en memoria del proceso%i . Se reemplazo la pagina %i por la pagina %i en la tlb \n",pcb_global.id, pagina_victima, numero_pagina);
			}

			if(tlb_llena()) {

				pagina_victima = elegir_victima_segun_algoritmo();
				//printf("Se elige la pagina %i como victima \n", pagina_victima);
				quitar_entrada_tlb(pagina_victima);
				//printf("Se quito la entrada de la pagina %i. Ahora hay %i entradas \n", pagina_victima, dictionary_size(tlb));
				actualizar_cola_de_algoritmo_por_reemplazo_hecho(pagina_victima, numero_pagina);
				log_trace(log_test, "No hubo reemplazo de pagina en memoria del proceso: %i . Se reemplazo la pagina %i por la pagina %i en la tlb \n", pcb_global.id,pagina_victima, numero_pagina);

			}else {

				if(!(*hubo_reemplazo_en_memoria))
					actualizar_cola_de_algoritmo_cuando_la_tlb_no_esta_llena(numero_pagina);

			}

			//Guardar página y marco en TLB
			//printf("Se agrega entrada a tlb \n");
			agregar_entrada_a_tlb(numero_pagina, marco);

			//Obtener dirección física
			direccion_fisica = marco * TAM_PAGINA + desplazamiento;
		}

		free(hubo_reemplazo_en_memoria);
		free(pagina_reemplazada_en_memoria);
		liberar_conexion(socket_memoria);
	}


	return direccion_fisica;
}

uint32_t validar_direccion_fisica(int socket_memoria, uint32_t pid, uint32_t numero_pagina,uint32_t desplazamiento) {
	t_paquete* paquete = armar_paquete_validacion_direccion_fisica(pid, numero_pagina,desplazamiento);
	enviar_paquete(paquete,socket_memoria);
	eliminar_paquete(paquete);

	void* buffer = malloc(sizeof(uint32_t));
	uint32_t respuesta;

	recv(socket_memoria, buffer, sizeof(uint32_t), MSG_WAITALL);
	memcpy(&respuesta, buffer, sizeof(uint32_t));
	log_debug(log_test, "Se recibio validacion de DF por parte de memoria del proceso: %i \n", pid);
	free(buffer);

	return respuesta;
}


void finalizar_cuenta_tiempo_ejecucion() {
	gettimeofday(&fin_ejecucion, NULL);
}

void calcular_tiempo_ejecucion() {
	tiempo_ejecucion = (fin_ejecucion.tv_sec - inicio_ejecucion.tv_sec) * 1000 + (fin_ejecucion.tv_usec - inicio_ejecucion.tv_usec)/1000;
}

bool pagina_esta_en_tlb(uint32_t numero_pagina) {

	bool retorno;
	char* nro_pagina =string_itoa(numero_pagina);
	if(dictionary_get(tlb, nro_pagina )== NULL) {
		retorno = false;
	}
	else {
		//printf("La pagina SI se encontro en la TLB \n");
		retorno = true;
	}
	free(nro_pagina);
	return retorno;
}

bool es_algoritmo(char* algoritmo) {
	return (bool) string_equals_ignore_case(algoritmo, REEMPLAZO_TLB);
}

uint32_t obtener_frame_de_tlb(uint32_t numero_pagina) {
	char* nro_pag = string_itoa(numero_pagina);
	//printf("Se va a obtener el marco para la pagina: %i \n", numero_pagina);
	uint32_t marco = *(uint32_t*)dictionary_get(tlb, nro_pag);
	free(nro_pag);
	return marco;
}

void agregar_entrada_a_tlb(uint32_t numero_pagina, uint32_t marco) {
	char* nro_pag = string_itoa(numero_pagina);
	uint32_t* aux = malloc(sizeof(uint32_t));
	*aux = marco;
	//printf("Se va a agregar la entrada: pagina [%i] marco[%i] \n", numero_pagina, marco);
	dictionary_put(tlb, nro_pag, (void*)aux);
	//uint32_t marco_obt = *(uint32_t*)dictionary_get(tlb, nro_pag);
	//printf("Se acaba de agregar la entrada pagina [%i] marco[%i] a la TLB \n", numero_pagina, marco_obt);
	//free(aux);
	free(nro_pag);
	log_info(log_test, "Se agrego a la TLB la entrada [pagina: %i, marco %i] PID:%i \n", numero_pagina, marco,pcb_global.id);
}

void quitar_entrada_tlb(uint32_t numero_pagina) {

	void destruir(uint32_t* marco){
		free(marco);
	}
	char* nro_pag = string_itoa(numero_pagina);
	dictionary_remove_and_destroy(tlb,nro_pag,(void*)destruir);
	free(nro_pag);
	//log_info(log_test, "Se quito de la TLB la entrada pagina: %i \n", numero_pagina);
}

void vaciar_tlb() {
	destruir_tlb();
	inicializar_tlb();
	//log_info(log_test, "La TLB fue vaciada porque hubo un process switch \n");
}

bool tlb_vacia() {
	return dictionary_is_empty(tlb);
}

bool tlb_llena() {
	return (uint32_t) dictionary_size(tlb) == ENTRADAS_TLB;
}

void actualizar_cola_de_algoritmo_por_referencia_a_una_pagina_en_tlb(uint32_t pagina_ultimamente_referenciada) {
	if(es_algoritmo("LRU")) {

		bool es_pagina_ultimamente_referenciada(uint32_t pagina) {
			return pagina == pagina_ultimamente_referenciada;
		}
		list_remove_by_condition(cola_lru, (void *) es_pagina_ultimamente_referenciada);
		list_add_in_index(cola_lru, 0, (void*) pagina_ultimamente_referenciada);
	}
}

uint32_t elegir_victima_segun_algoritmo() {
	uint32_t pagina_victima;

	if(es_algoritmo("FIFO")) {
		pagina_victima = elegir_victima_segun_fifo();
	}
	if(es_algoritmo("LRU")) {
		pagina_victima = elegir_victima_segun_lru();
	}

	return pagina_victima;
}

void actualizar_cola_de_algoritmo_por_reemplazo_hecho(uint32_t pagina_victima, uint32_t pagina_nueva) {
	if(es_algoritmo("FIFO")) {
		(uint32_t) queue_pop(cola_fifo);
		queue_push(cola_fifo, (void*) pagina_nueva);
	}
	if(es_algoritmo("LRU")) {
		//Buscar y borrar la pagina victima e insertar en la posición 0 la nueva página
		list_remove(cola_lru, ENTRADAS_TLB -1);
		list_add_in_index(cola_lru, 0, (void*) pagina_nueva);
	}
}

void actualizar_cola_de_algoritmo_para_dejar_tlb_consistente_por_reemplazo_hecho_en_memoria(uint32_t pagina_victima, uint32_t pagina_nueva) {
	bool es_pagina_victima(uint32_t pagina) {
		return pagina == pagina_victima;
	}
	if(es_algoritmo("FIFO")) {
		list_remove_by_condition(cola_fifo->elements, (void *) es_pagina_victima);
		queue_push(cola_fifo, (void*) pagina_nueva);
	}
	if(es_algoritmo("LRU")) {
		list_remove_by_condition(cola_lru, (void *) es_pagina_victima);
		list_add_in_index(cola_lru, 0, (void*) pagina_nueva);
	}
}

void actualizar_cola_de_algoritmo_cuando_la_tlb_no_esta_llena(uint32_t pagina_nueva) {
	if(es_algoritmo("FIFO")) {
		queue_push(cola_fifo, (void*) pagina_nueva);
	}
	if(es_algoritmo("LRU")) {
		list_add_in_index(cola_lru, 0, (void*) pagina_nueva);
	}
}

uint32_t elegir_victima_segun_fifo() {
	return (uint32_t) queue_peek(cola_fifo);
}

uint32_t elegir_victima_segun_lru() {
	return (uint32_t) list_get(cola_lru, ENTRADAS_TLB-1);
}

uint32_t obtener_frame_de_tabla_de_nivel_2(int socket_memoria, uint32_t tabla_nivel_1, uint32_t entrada_tabla_1er_nivel, uint32_t entrada_tabla_2do_nivel, uint32_t* hubo_reemplazo_en_memoria, uint32_t* pagina_reemplazada_en_memoria) {
	uint32_t offset = 0;
	void* buffer = malloc(sizeof(uint32_t)*2);

	//----------PRIMER ACCESO----------

	// Enviar mensaje a memoria con entrada_tabla_1er_nivel y tabla_nivel_1 para conocer en que tabla de 2do nivel está direccionado el marco en el que se encuentra la página a la que queremos acceder
	memcpy(buffer, &tabla_nivel_1, sizeof(uint32_t));
	offset = offset + sizeof(uint32_t);
	memcpy(buffer + offset, &entrada_tabla_1er_nivel, sizeof(uint32_t));
	offset = offset + sizeof(uint32_t);
	log_trace(log_test,"--------------PID:%i , tabla nvl1: %i, entrada nvl1: %i \n",pcb_global.id,tabla_nivel_1,entrada_tabla_1er_nivel);
	send(socket_memoria, buffer, sizeof(uint32_t)*2, 0);
	free(buffer);

	// Recibir de memoria ID_tabla_2do_nivel
	buffer= malloc(sizeof(uint32_t));

	uint32_t tabla_nivel_2 = 3; // TODO: Borrar cuando funcione
	recv(socket_memoria, buffer, sizeof(uint32_t), MSG_WAITALL);
	memcpy(&tabla_nivel_2,buffer,sizeof(uint32_t));
	log_trace(log_test,"el numero de tabla de pagina de segundo nivel es: %i\n", tabla_nivel_2);
	free(buffer);
	// Enviar a memoria entrada_tabla_2do_nivel

	//----------PRIMER ACCESO----------




	//----------SEGUNDO ACCESO----------

	void* acceso_tabla_2do_nivel = malloc(sizeof(uint32_t)*2);

	acceso_a_tabla_2do_nivel(acceso_tabla_2do_nivel, entrada_tabla_2do_nivel, tabla_nivel_2);
	log_debug(log_test,"El numero de entrada de tabla de 2do nivel es: %i y tabla es: %i \n",entrada_tabla_2do_nivel,tabla_nivel_2);
	send(socket_memoria,acceso_tabla_2do_nivel,sizeof(uint32_t)*2,0);

	free(acceso_tabla_2do_nivel);

	// Recibir numero de marco y llenar TLB
	int numero_marco;
	buffer = malloc(sizeof(uint32_t)*3);
	recv(socket_memoria, buffer, sizeof(uint32_t)*3, MSG_WAITALL);
	memcpy(&numero_marco,buffer,sizeof(uint32_t));
	offset = sizeof(uint32_t);
	memcpy(hubo_reemplazo_en_memoria, buffer + offset, sizeof(uint32_t));
	offset = offset + sizeof(uint32_t);
	memcpy(pagina_reemplazada_en_memoria, buffer + offset, sizeof(uint32_t));
	offset = offset + sizeof(uint32_t);

	//----------SEGUNDO ACCESO----------

	//printf("Hubo reemplazo: %i\n", *hubo_reemplazo_en_memoria);

	//log_debug(log_global, "Se obtuvo un numero de marco para una traduccion \n");
	//log_info(log_test, "El numero de marco obtenido es: %i\n", numero_marco);
	log_error(log_test, "Hubo reemplazo en memoria? (1-> SI, 0->NO): %i\n", *hubo_reemplazo_en_memoria);

	if(*hubo_reemplazo_en_memoria){
		//log_info(log_test, "Pagina reemplazada: %i\n", *pagina_reemplazada_en_memoria);
	}

	free(buffer);

	return numero_marco;
}






