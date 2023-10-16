#include "planificacion.h"

void iniciar_colecciones() {
	cola_new = list_create();
	cola_ready = list_create();
	cola_execute = list_create();
	cola_suspended_ready = list_create();
	cola_blocked = list_create();
	cola_exit = list_create();
}

void iniciar_semaforos() {

	// Semaforos mutex para las colas de estados del proceso

	if(pthread_mutex_init(&sem_cola_new, NULL) != 0) {
		pthread_mutex_lock(&sem_log_global);
		log_error(log_global, "Error creando el mutex para la cola NEW");
		pthread_mutex_unlock(&sem_log_global);
	}

	if(pthread_mutex_init(&sem_cola_ready, NULL) != 0) {
		pthread_mutex_lock(&sem_log_global);
		log_error(log_global, "Error creando el mutex para la cola READY");
		pthread_mutex_unlock(&sem_log_global);
	}

	if(pthread_mutex_init(&sem_cola_exec, NULL) != 0) {
		pthread_mutex_lock(&sem_log_global);
		log_error(log_global, "Error creando el mutex para la cola EXECUTE");
		pthread_mutex_unlock(&sem_log_global);
	}

	if(pthread_mutex_init(&sem_cola_susp_ready, NULL) != 0) {
		pthread_mutex_lock(&sem_log_global);
		log_error(log_global, "Error creando el mutex para la cola SUSPENDED READY");
		pthread_mutex_unlock(&sem_log_global);
	}

	if(pthread_mutex_init(&sem_cola_blocked, NULL) != 0) {
		pthread_mutex_lock(&sem_log_global);
		log_error(log_global, "Error creando el mutex para la cola BLOCKED");
		pthread_mutex_unlock(&sem_log_global);
	}

	if(pthread_mutex_init(&sem_cola_exit, NULL) != 0) {
		pthread_mutex_lock(&sem_log_global);
		log_error(log_global, "Error creando el mutex para la cola EXIT");
		pthread_mutex_unlock(&sem_log_global);
	}

	// Semaforos mutex para variables compartidas

	if(pthread_mutex_init(&sem_ids_procesos, NULL) != 0) {
		pthread_mutex_lock(&sem_log_global);
		log_error(log_global, "Error creando el mutex para sincronizar la asignacion del ID de proceso.");
		pthread_mutex_unlock(&sem_log_global);
	}

	if(pthread_mutex_init(&sem_lista_procesos, NULL) != 0) {
		pthread_mutex_lock(&sem_log_global);
		log_error(log_global, "Error creando el mutex para sincronizar la lista global de procesos.");
		pthread_mutex_unlock(&sem_log_global);
	}

	if(pthread_mutex_init(&sem_hay_proceso_ejecutando, NULL) != 0) {
		pthread_mutex_lock(&sem_log_global);
		log_error(log_global, "Error creando el mutex para sincronizar la variable hay_proceso_ejecutando.");
		pthread_mutex_unlock(&sem_log_global);
	}

	if(pthread_mutex_init(&sem_conexiones, NULL) != 0) {
		pthread_mutex_lock(&sem_log_global);
		log_error(log_global, "Error creando el mutex para sincronizar las conexiones con Memoria y CPU.");
		pthread_mutex_unlock(&sem_log_global);
	}

	if(pthread_mutex_init(&sem_estado_proceso_blocked, NULL) != 0) {
		pthread_mutex_lock(&sem_log_global);
		log_error(log_global, "Error creando el mutex para sincronizar el estado blocked/suspended blocked de los procesos.");
		pthread_mutex_unlock(&sem_log_global);
	}

	sem_nuevo_proceso = malloc(sizeof(sem_t));
	sem_init(sem_nuevo_proceso, 1, 0);

	sem_grado_multiprogramacion = malloc(sizeof(sem_t));
	sem_init(sem_grado_multiprogramacion, 1, GRADO_MULTIPROGRAMACION);

	sem_hay_procesos_ready = malloc(sizeof(sem_t));
	sem_init(sem_hay_procesos_ready, 1, 0);

	sem_hay_proceso_blocked = malloc(sizeof(sem_t));
	sem_init(sem_hay_proceso_blocked, 1, 0);

}

void iniciar_estructuras_procesos() {
	ids_procesos = 1;
	procesos = list_create();
	hay_proceso_ejecutando = 0;
	ejecutar_planificadores = 0;

	iniciar_semaforos();
	iniciar_colecciones();
}

void destruir_instruccion(t_instruccion* instruccion) {

	free(instruccion);
}

void destruir_proceso(t_pcb* proceso) {

	list_destroy_and_destroy_elements(proceso->instrucciones, (void*)destruir_instruccion);
	free(proceso);
}

void destruir_colecciones() {
	list_destroy(cola_new);
	list_destroy(cola_ready);
	list_destroy(cola_execute);
	list_destroy(cola_suspended_ready);
	list_destroy(cola_blocked);
	list_destroy(cola_exit);

	list_destroy_and_destroy_elements(procesos, (void*)destruir_proceso); // destruye la lista global de procesos
}

void destruir_semaforos() {
	pthread_mutex_destroy(&sem_cola_new);
	pthread_mutex_destroy(&sem_cola_ready);
	pthread_mutex_destroy(&sem_cola_exec);
	pthread_mutex_destroy(&sem_cola_susp_ready);
	pthread_mutex_destroy(&sem_cola_blocked);
	pthread_mutex_destroy(&sem_cola_exit);

	pthread_mutex_destroy(&sem_ids_procesos);
	pthread_mutex_destroy(&sem_lista_procesos);
	pthread_mutex_destroy(&sem_hay_proceso_ejecutando);

	pthread_mutex_destroy(&sem_conexiones); 
	pthread_mutex_destroy(&sem_estado_proceso_blocked);

	sem_destroy(sem_nuevo_proceso);
	sem_destroy(sem_hay_procesos_ready);
	sem_destroy(sem_hay_proceso_blocked);
	sem_destroy(sem_grado_multiprogramacion);
}

void destruir_estructuras_procesos() {
	destruir_semaforos();
	destruir_colecciones();
}

// -------------------------------------------- FUNCIONES PROCESOS -------------------------------------------- //

void crear_proceso(t_list* instrucciones, uint32_t tamanio, int socket_consola) {
	t_pcb* proceso = malloc(sizeof(t_pcb));

	pthread_mutex_lock(&sem_ids_procesos);
	proceso->ID = ids_procesos;
	ids_procesos++;
	pthread_mutex_unlock(&sem_ids_procesos);

	proceso->tamanio = tamanio;
	proceso->estado_actual = NEW;
	proceso->instrucciones = list_create();
	proceso->program_counter = 0;
	proceso->rafaga_real_anterior = 0.0;
	proceso->estimacion_anterior = ESTIMACION_INICIAL;
	proceso->estimacion_rafaga = estimar_proxima_rafaga(proceso);
	proceso->tiempo_bloqueado = 0;
	proceso->tiempo_acumulado = 0.0;
	proceso->socket_consola = socket_consola;

	inicializar_lista_instrucciones(proceso, instrucciones);

	pthread_mutex_lock(&sem_cola_new);
	list_add(cola_new, proceso);
	pthread_mutex_unlock(&sem_cola_new);

	pthread_mutex_lock(&sem_lista_procesos);
	list_add(procesos, proceso);
	pthread_mutex_unlock(&sem_lista_procesos);

	pthread_mutex_lock(&sem_log_global);
	log_info(log_global, "Estructuras del proceso %i iniciadas correctamente. Estado actual: NEW.", proceso->ID);
	pthread_mutex_unlock(&sem_log_global);

}

void ejecutar_planificador_largo_plazo() { // Verifica el grado de multiprogramacion para ingresar procesos a memoria

	while(ejecutar_planificadores) {

		sem_wait(sem_nuevo_proceso); // Solo pasa cuando llega un proceso nuevo a Kernel por parte de Consola
		sem_wait(sem_grado_multiprogramacion);

		pthread_mutex_lock(&sem_cola_susp_ready);
		int no_hay_proceso_susp_listo = list_is_empty(cola_suspended_ready);
		pthread_mutex_unlock(&sem_cola_susp_ready);

		if(no_hay_proceso_susp_listo == 0) { // Hay procesos en la cola suspended_ready que tienen mas prioridad que los de new
			pthread_mutex_lock(&sem_cola_susp_ready);
			t_pcb* proceso = list_get(cola_suspended_ready, 0); // Saca un proceso segun FIFO
			pthread_mutex_unlock(&sem_cola_susp_ready);

			cambiar_estado_proceso(proceso, READY);

		} else {

			pthread_mutex_lock(&sem_cola_new);
			int no_hay_proceso_nuevo = list_is_empty(cola_new);
			pthread_mutex_unlock(&sem_cola_new);

			if(no_hay_proceso_nuevo == 0) { // Hay procesos en la cola new
				pthread_mutex_lock(&sem_cola_new);
				t_pcb* proceso = list_get(cola_new, 0); // Saca un proceso segun FIFO
				pthread_mutex_unlock(&sem_cola_new);

				// Le pedimos a memoria que nos asigne una tabla de paginas antes de pasarlo a READY
				int socket_memoria = conectar_con_servidor(IP_MEMORIA, PUERTO_MEMORIA);
				t_paquete* paquete = paquete_pedido_tabla_de_paginas(proceso->ID, proceso->tamanio);
				enviar_paquete(paquete, socket_memoria);
				eliminar_paquete(paquete);

				int32_t numero_tabla = -1;

				int return_recv = recv(socket_memoria, &numero_tabla, sizeof(uint32_t), MSG_WAITALL);

				if(return_recv != -1) {

					if(numero_tabla >= 0) { // NUMERO_TABLA_PAGINAS
						proceso->tabla_paginas = numero_tabla;
						cambiar_estado_proceso(proceso, READY); // Una vez recibida la confirmacion y el nro de tabla, lo pasamos a READY
					}

				} else {
					pthread_mutex_lock(&sem_log_global);
					log_error(log_global, "Error al recibir el numero de la tabla de paginas para el proceso %i.", proceso->ID);
					pthread_mutex_unlock(&sem_log_global);
				}

				liberar_conexion(socket_memoria);
			}

		}

	}

}

void ejecutar_planificador_corto_plazo() {

	while(ejecutar_planificadores) {

		sem_wait(sem_hay_procesos_ready);

		pthread_mutex_lock(&sem_cola_ready);
		t_pcb* proceso_a_ejecutar = proximo_proceso_a_ejecutar(); // Toma un proceso de READY segun el algoritmo seteado
		pthread_mutex_unlock(&sem_cola_ready);

		pthread_mutex_lock(&sem_log_global);
		log_info(log_global, "Proximo proceso a ejecutar: %i", proceso_a_ejecutar->ID);
		pthread_mutex_unlock(&sem_log_global);

		pthread_mutex_lock(&sem_conexiones);
		int socket_cpu_dispatch = conectar_con_servidor(IP_CPU, PUERTO_CPU_DISPATCH);
		pthread_mutex_unlock(&sem_conexiones);

		t_paquete* paquete = paquete_proceso_a_ejecutar(proceso_a_ejecutar); // Enviamos a CPU el proceso a ejecutar
		enviar_paquete(paquete, socket_cpu_dispatch);
		eliminar_paquete(paquete);

		cambiar_estado_proceso(proceso_a_ejecutar, EXEC);

		pthread_mutex_lock(&sem_hay_proceso_ejecutando);
		hay_proceso_ejecutando = 1; // Indicamos que hay un proceso ejecutando para que asi no se envie una interrupcion sin la cpu en uso
		pthread_mutex_unlock(&sem_hay_proceso_ejecutando);

		op_code codigo_operacion;
		if(recv(socket_cpu_dispatch, &codigo_operacion, sizeof(op_code), MSG_WAITALL) != -1) { // Quedamos a la espera de que cpu nos diga si termino, se bloqueo o hubo una interrupcion

			int size;
			void* buffer = recibir_buffer(&size, socket_cpu_dispatch);

			if(codigo_operacion == PROCESO_BLOQUEADO) {
				t_pcb* proceso_bloqueado = deserializar_paquete_proceso_bloqueado(buffer);

				pthread_mutex_lock(&sem_hay_proceso_ejecutando);
				hay_proceso_ejecutando = 0;
				pthread_mutex_unlock(&sem_hay_proceso_ejecutando);

				cambiar_estado_proceso(proceso_bloqueado, BLOCKED);

			} else if(codigo_operacion == PROCESO_TERMINADO) {
				t_pcb* proceso_terminado = deserializar_paquete_proceso_terminado(buffer);

				pthread_mutex_lock(&sem_hay_proceso_ejecutando);
				hay_proceso_ejecutando = 0;
				pthread_mutex_unlock(&sem_hay_proceso_ejecutando);

				cambiar_estado_proceso(proceso_terminado, EXIT);

			} else if(codigo_operacion == PROCESO_DESALOJADO) { // CPU al recibir una interrupcion usa la misma conexion para enviar el pcb
				t_pcb* proceso_desalojado = deserializar_paquete_proceso_desalojado(buffer);

				pthread_mutex_lock(&sem_hay_proceso_ejecutando);
				hay_proceso_ejecutando = 0;
				pthread_mutex_unlock(&sem_hay_proceso_ejecutando);

				cambiar_estado_proceso(proceso_desalojado, READY); // Vuelve a la cola READY para que al tomar el prox proceso a ejecutar lo tenga en cuenta (reestimar)

			} else {
				pthread_mutex_lock(&sem_log_global);
				log_error(log_global, "El codigo de operacion enviado por CPU es erroneo!");
				pthread_mutex_unlock(&sem_log_global);
			}

			free(buffer);
			liberar_conexion(socket_cpu_dispatch);

		} else {
			pthread_mutex_lock(&sem_log_global);
			log_error(log_global, "Error al recibir el resultado de la ejecucion del proceso %i por parte de CPU.", proceso_a_ejecutar->ID);
			pthread_mutex_unlock(&sem_log_global);
		}

	}

}

void interrumpir_ciclo_de_instruccion() {

	pthread_mutex_lock(&sem_conexiones);
	int socket_cpu_interrupt = conectar_con_servidor(IP_CPU, PUERTO_CPU_INTERRUPT);
	pthread_mutex_unlock(&sem_conexiones);

	t_paquete* paquete = paquete_aviso_interrupcion();

	enviar_paquete(paquete, socket_cpu_interrupt);
	eliminar_paquete(paquete);
	liberar_conexion(socket_cpu_interrupt);

	// Solo enviamos la interrupcion por esta conexion, el pcb lo recibimos por la conexion DISPATCH.
}

void ejecutar_planificador_mediano_plazo() {

	while(ejecutar_planificadores) {

		sem_wait(sem_hay_proceso_blocked);

		pthread_mutex_lock(&sem_cola_blocked);
		t_pcb* proceso_bloqueado = list_get(cola_blocked, 0);
		pthread_mutex_unlock(&sem_cola_blocked);

		pthread_mutex_lock(&sem_log_global);
		log_info(log_global, "Tiempo que se debe bloquear el proceso %i: %i milisegundos", proceso_bloqueado->ID, proceso_bloqueado->tiempo_bloqueado);
		pthread_mutex_unlock(&sem_log_global);

		usleep(1000 * proceso_bloqueado->tiempo_bloqueado);

		pthread_mutex_lock(&sem_estado_proceso_blocked);
		if(proceso_bloqueado->estado_actual == BLOCKED) {
			cambiar_estado_proceso(proceso_bloqueado, READY);
			pthread_kill(proceso_bloqueado->thread_control_suspension, 0);

		} else if(proceso_bloqueado->estado_actual == SUSPENDED_BLOCKED) {
			cambiar_estado_proceso(proceso_bloqueado, SUSPENDED_READY);
			pthread_detach(proceso_bloqueado->thread_control_suspension);
		}
		pthread_mutex_unlock(&sem_estado_proceso_blocked);

	}

}

void verificar_tiempo_de_bloqueo(t_pcb* proceso_bloqueado) {

	usleep(1000 * TIEMPO_MAXIMO_BLOQUEADO);

	pthread_mutex_lock(&sem_estado_proceso_blocked);
	if(proceso_bloqueado->estado_actual == BLOCKED) {
		cambiar_estado_proceso(proceso_bloqueado, SUSPENDED_BLOCKED);
	}
	pthread_mutex_unlock(&sem_estado_proceso_blocked);

}

// ------------------------------------------------------- ALGORITMOS ------------------------------------------------------- //

float estimar_proxima_rafaga(t_pcb* proceso) {

	// FORMULA: Prox. Estimacion = Alfa * Rafaga Real Ant. + (1-Alfa) * Estimacion Ant.
	return (ALFA * (proceso->rafaga_real_anterior)) + ((1 - ALFA) * (proceso->estimacion_anterior));
}

t_pcb* menor_proxima_rafaga(t_pcb* un_proceso, t_pcb* otro_proceso) {

	// Si son iguales, desempata por FIFO.
	return un_proceso->estimacion_rafaga <= otro_proceso->estimacion_rafaga ? un_proceso : otro_proceso;
}

t_pcb* proximo_proceso_a_ejecutar() {

	if(strcmp(ALGORITMO_PLANIFICACION,"SRT") == 0) { // SJF con desalojo
		return list_get_minimum(cola_ready, (void*)menor_proxima_rafaga);

	} else { // FIFO
		return list_get(cola_ready, 0);
	}

}

// -------------------------------------------------- FUNCIONES AUXILIARES -------------------------------------------------- //

void inicializar_lista_instrucciones(t_pcb* proceso, t_list* lista_instrucciones) {
	t_list_iterator* iterador_instrucciones = list_iterator_create(lista_instrucciones);

	while(list_iterator_has_next(iterador_instrucciones)) {
		t_instruccion* instruccion = list_iterator_next(iterador_instrucciones);

		if(string_equals_ignore_case(instruccion->identificador, "NO_OP")) {

			for(int i=0; i<instruccion->parametro_1; i++) {
				t_instruccion* instruccion_no_op = malloc(sizeof(t_instruccion));
				char identificador[6];

				memcpy(identificador, instruccion->identificador, strlen(instruccion->identificador)+1);
				memcpy(instruccion_no_op->identificador, identificador, strlen(identificador) + 1);

				instruccion_no_op->parametro_1 = 0;
				instruccion_no_op->parametro_2 = 0;

				list_add(proceso->instrucciones, instruccion_no_op);
			}

		} else {
			list_add(proceso->instrucciones, instruccion);
		}

	}

	list_iterator_destroy(iterador_instrucciones);
}

t_pcb* get_proceso(uint32_t proceso_id) {
	t_pcb* result = NULL;

	void buscar_proceso(t_pcb* un_proceso) {
		if(un_proceso->ID == proceso_id)
			result = un_proceso;
	}

	pthread_mutex_lock(&sem_lista_procesos);
	list_iterate(procesos, (void*)buscar_proceso);
	pthread_mutex_unlock(&sem_lista_procesos);

	return result;
}

void cambiar_estado_proceso(t_pcb* proceso, estado nuevo_estado) {
	t_pcb* proceso_a_cambiar; // El proceso que retorna la funcion list_remove_by_condition
	char* estado_anterior; // Para asi loggear de que estado viene el proceso y no repetirlo varias veces
	int socket_memoria;
	op_code confirmacion;
	t_paquete* paquete_memoria;
	t_paquete* paquete_consola;

	bool mismo_id(t_pcb* otro_proceso) {
		return (proceso->ID == otro_proceso->ID);
	}

	switch(nuevo_estado) {

	// NEW: Este estado se maneja al iniciar un proceso nuevo

	case 2: // READY

		if(proceso->estado_actual == NEW) {
			pthread_mutex_lock(&sem_cola_new);
			proceso_a_cambiar = list_remove_by_condition(cola_new, (void*)mismo_id);
			pthread_mutex_unlock(&sem_cola_new);

			estado_anterior = "NEW";

		} else if(proceso->estado_actual == SUSPENDED_READY) {
			pthread_mutex_lock(&sem_cola_susp_ready);
			proceso_a_cambiar = list_remove_by_condition(cola_suspended_ready, (void*)mismo_id);
			pthread_mutex_unlock(&sem_cola_susp_ready);

			estado_anterior = "SUSPENDED_READY";

		} else if(proceso->estado_actual == EXEC) {
			pthread_mutex_lock(&sem_cola_exec);
			proceso_a_cambiar = list_remove_by_condition(cola_execute, (void*)mismo_id);
			pthread_mutex_unlock(&sem_cola_exec);

			estado_anterior = "EXECUTE";

		} else if(proceso->estado_actual == BLOCKED) {
			pthread_mutex_lock(&sem_cola_blocked);
			proceso_a_cambiar = list_remove_by_condition(cola_blocked, (void*)mismo_id);
			pthread_mutex_unlock(&sem_cola_blocked);

			estado_anterior = "BLOCKED";
		}

		pthread_mutex_lock(&sem_cola_ready);
		list_add(cola_ready, proceso_a_cambiar);
		pthread_mutex_unlock(&sem_cola_ready);

		proceso_a_cambiar->estado_actual = READY;

		pthread_mutex_lock(&sem_log_global);
		log_info(log_global, "Proceso %i movido de la cola %s a la cola READY.", proceso_a_cambiar->ID, estado_anterior);
		pthread_mutex_unlock(&sem_log_global);

		// Si el algoritmo es SJF y hay un proceso haciendo uso de la CPU, podemos interrumpir la ejecucion...
		if(string_equals_ignore_case(ALGORITMO_PLANIFICACION, "SRT")) {

			pthread_mutex_lock(&sem_hay_proceso_ejecutando);
			if(hay_proceso_ejecutando) {
				interrumpir_ciclo_de_instruccion();
			}
			pthread_mutex_unlock(&sem_hay_proceso_ejecutando);

		}

		sem_post(sem_hay_procesos_ready); // Le indico al plani de corto plazo que hay un nuevo proceso para ejecutar

		break;

	case 3: // EXECUTE

		pthread_mutex_lock(&sem_cola_ready);
		proceso_a_cambiar = list_remove_by_condition(cola_ready, (void*)mismo_id);
		pthread_mutex_unlock(&sem_cola_ready);

		pthread_mutex_lock(&sem_cola_exec);
		list_add(cola_execute, proceso_a_cambiar);
		pthread_mutex_unlock(&sem_cola_exec);

		proceso_a_cambiar->estado_actual = EXEC;

		pthread_mutex_lock(&sem_log_global);
		log_info(log_global, "Proceso %i movido de la cola READY a la cola EXECUTE.", proceso_a_cambiar->ID);
		pthread_mutex_unlock(&sem_log_global);

		break;

	case 4: // BLOCKED

		pthread_mutex_lock(&sem_cola_exec);
		proceso_a_cambiar = list_remove_by_condition(cola_execute, (void*)mismo_id);
		pthread_mutex_unlock(&sem_cola_exec);

		pthread_mutex_lock(&sem_cola_blocked);
		list_add(cola_blocked, proceso);
		pthread_mutex_unlock(&sem_cola_blocked);

		proceso->estado_actual = BLOCKED;

		pthread_create(&(proceso->thread_control_suspension), NULL, (void*)verificar_tiempo_de_bloqueo, proceso);

		pthread_mutex_lock(&sem_log_global);
		log_info(log_global, "Proceso %i movido de la cola EXECUTE a la cola BLOCKED.", proceso->ID);
		pthread_mutex_unlock(&sem_log_global);

		sem_post(sem_hay_proceso_blocked);

		break;

	case 5: // SUSPENDED_BLOCKED

		socket_memoria = conectar_con_servidor(IP_MEMORIA, PUERTO_MEMORIA);
		paquete_memoria = paquete_aviso_suspension_proceso(proceso->ID);
		enviar_paquete(paquete_memoria, socket_memoria);
		eliminar_paquete(paquete_memoria);

		recv(socket_memoria, &confirmacion, sizeof(op_code), MSG_WAITALL);

		if(confirmacion == PROCESO_SUSPENDIDO) { 
			proceso->estado_actual = SUSPENDED_BLOCKED; // Una vez recibida la confirmacion, lo pasamos a SUSPENDED_BLOCKED
		}

		liberar_conexion(socket_memoria);

		sem_post(sem_grado_multiprogramacion);

		pthread_mutex_lock(&sem_log_global);
		log_info(log_global, "El proceso %i cambió su estado de BLOCKED a SUSPENDED_BLOCKED por estar más de %i milisegundos bloqueado.", proceso->ID, TIEMPO_MAXIMO_BLOQUEADO);
		pthread_mutex_unlock(&sem_log_global);

		break;

	case 6: // SUSPENDED_READY

		pthread_mutex_lock(&sem_cola_blocked);
		proceso_a_cambiar = list_remove_by_condition(cola_blocked, (void*)mismo_id);
		pthread_mutex_unlock(&sem_cola_blocked);

		pthread_mutex_lock(&sem_cola_susp_ready);
		list_add(cola_suspended_ready, proceso_a_cambiar);
		pthread_mutex_unlock(&sem_cola_susp_ready);

		proceso_a_cambiar->estado_actual = SUSPENDED_READY;

		pthread_mutex_lock(&sem_log_global);
		log_info(log_global, "Proceso %i en estado SUSPENDED_BLOCKED fue movido de la cola BLOCKED a la cola SUSPENDED_READY.", proceso->ID);
		pthread_mutex_unlock(&sem_log_global);

		sem_post(sem_nuevo_proceso); // Para que el plani de largo plazo pase el proceso a READY

		break;

	case 7: // EXIT

		if(proceso->estado_actual == NEW) {
			pthread_mutex_lock(&sem_cola_new);
			proceso_a_cambiar = list_remove_by_condition(cola_new, (void*)mismo_id);
			pthread_mutex_unlock(&sem_cola_new);

			estado_anterior = "NEW";

		} else if(proceso->estado_actual == READY) {
			pthread_mutex_lock(&sem_cola_ready);
			proceso_a_cambiar = list_remove_by_condition(cola_ready, (void*)mismo_id);
			pthread_mutex_lock(&sem_cola_ready);

			estado_anterior = "READY";

		} else if(proceso->estado_actual == EXEC) {
			pthread_mutex_lock(&sem_cola_exec);
			proceso_a_cambiar = list_remove_by_condition(cola_execute, (void*)mismo_id);
			pthread_mutex_unlock(&sem_cola_exec);

			estado_anterior = "EXECUTE";

		} else if(proceso->estado_actual == BLOCKED) {
			pthread_mutex_lock(&sem_cola_blocked);
			proceso_a_cambiar = list_remove_by_condition(cola_blocked, (void*)mismo_id);
			pthread_mutex_unlock(&sem_cola_blocked);

			estado_anterior = "BLOCKED";

		} else if(proceso->estado_actual == SUSPENDED_BLOCKED) {
			pthread_mutex_lock(&sem_cola_blocked);
			proceso_a_cambiar = list_remove_by_condition(cola_blocked, (void*)mismo_id);
			pthread_mutex_unlock(&sem_cola_blocked);

			estado_anterior = "SUSPENDED_BLOCKED";

		} else if(proceso->estado_actual == SUSPENDED_READY) {
			pthread_mutex_lock(&sem_cola_susp_ready);
			proceso_a_cambiar = list_remove_by_condition(cola_suspended_ready, (void*)mismo_id);
			pthread_mutex_unlock(&sem_cola_susp_ready);

			estado_anterior = "SUSPENDED_READY";
		}

		pthread_mutex_lock(&sem_conexiones);
		socket_memoria = conectar_con_servidor(IP_MEMORIA, PUERTO_MEMORIA);
		pthread_mutex_unlock(&sem_conexiones);

		paquete_memoria = paquete_aviso_fin_proceso_memoria(proceso->ID); // Le enviamos ID del proceso + codigo de operacion
		enviar_paquete(paquete_memoria, socket_memoria);
		eliminar_paquete(paquete_memoria);

		recv(socket_memoria, &confirmacion, sizeof(op_code), MSG_WAITALL);

		if(confirmacion == ESTRUCTURAS_LIBERADAS) {

			paquete_consola = paquete_aviso_fin_proceso_consola();
			enviar_paquete(paquete_consola, proceso_a_cambiar->socket_consola);
			eliminar_paquete(paquete_consola);
			liberar_conexion(proceso_a_cambiar->socket_consola);

			list_add(cola_exit, proceso_a_cambiar);
			proceso_a_cambiar->estado_actual = EXIT;

			pthread_mutex_lock(&sem_log_global);
			log_info(log_global, "Proceso %i movido de la cola %s a la cola EXIT.", proceso_a_cambiar->ID, estado_anterior);
			pthread_mutex_unlock(&sem_log_global);

			sem_post(sem_grado_multiprogramacion);

		} else {

			pthread_mutex_lock(&sem_log_global);
			log_error(log_global, "Error al liberar la estructuras del proceso %i en Memoria.", proceso_a_cambiar->ID);
			pthread_mutex_unlock(&sem_log_global);

		}

		liberar_conexion(socket_memoria);

		break;

	default:

		pthread_mutex_lock(&sem_log_global);
		log_error(log_global, "El estado indicado no existe!");
		pthread_mutex_unlock(&sem_log_global);

		break;
	}

}

// ----------------------------------- FUNCIONES PAQUETES (Deserializacion/Serializacion) ------------------------------------ //

t_paquete* paquete_proceso_a_ejecutar(t_pcb* proceso) { // Paquete a enviar a CPU cuando ponemos un proceso a ejecutar
	t_buffer* buffer = malloc(sizeof(t_buffer));
	t_paquete* paquete = malloc(sizeof(t_paquete));
	uint32_t offset = 0; // Desplazamiento

	buffer->size = sizeof(uint32_t)*4 + sizeof(estado) + sizeof(int32_t) + sizeof(float);

	int contador = 0;
	uint32_t tamanio_instruccion;
	while(contador < list_size(proceso->instrucciones)) {
		t_instruccion* instruccion = list_get(proceso->instrucciones, contador);

		tamanio_instruccion = sizeof(instruccion->identificador) + sizeof(uint32_t) + sizeof(uint32_t);
		buffer->size += tamanio_instruccion;

		contador++;
	}

	void* stream = malloc(buffer->size);

	memcpy(stream + offset, &(proceso->ID), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &(proceso->tamanio), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &(proceso->estado_actual), sizeof(estado));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &(proceso->instrucciones->elements_count), sizeof(int32_t)); // Para que KERNEL sepa cuantos elementos va a leer.
	offset += sizeof(int32_t);

	contador = 0;
	while(contador < list_size(proceso->instrucciones)) {
		t_instruccion* instruccion = list_get(proceso->instrucciones, contador);

		memcpy(stream + offset, &(instruccion->identificador), sizeof(instruccion->identificador));
		offset += sizeof(instruccion->identificador);
		memcpy(stream + offset, &(instruccion->parametro_1), sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(stream + offset, &(instruccion->parametro_2), sizeof(uint32_t));
		offset += sizeof(uint32_t);

		contador++;
	}

	memcpy(stream + offset, &(proceso->program_counter), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &(proceso->tabla_paginas), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &(proceso->estimacion_rafaga), sizeof(float));
	offset += sizeof(float);

	buffer->stream = stream;
	paquete->codigo_operacion = PROCESO_A_EJECUTAR;
	paquete->buffer = buffer;

	return paquete;
}

t_pcb* deserializar_paquete_proceso_desalojado(void* buffer) {
	int desplazamiento = 0;
	float tiempo_ejecutado;
	uint32_t ID_proceso;

	memcpy(&ID_proceso, buffer + desplazamiento, sizeof(uint32_t)); // id del proceso para buscarlo en la lista global
	desplazamiento += sizeof(uint32_t);

	t_pcb* proceso = get_proceso(ID_proceso);

	memcpy(&tiempo_ejecutado, buffer + desplazamiento, sizeof(float)); // milisegundos que ejecuto el proceso
	desplazamiento += sizeof(float);

	memcpy(&proceso->program_counter, buffer + desplazamiento, sizeof(uint32_t)); // actualizamos el PC
	desplazamiento += sizeof(uint32_t);

	proceso->tiempo_acumulado = proceso->tiempo_acumulado + tiempo_ejecutado;
	proceso->estimacion_rafaga = proceso->estimacion_rafaga - tiempo_ejecutado;

//	printf("\n\nEl proceso %i fue desalojado\n", proceso->ID);
//	printf("\nTiempo que ejecuto el proceso %i: %f rafagas de cpu\n", proceso->ID, tiempo_ejecutado);
//	printf("\nActualizacion de la rafaga del proceso %i: %f rafagas de cpu\n\n", proceso->ID, proceso->estimacion_rafaga);

	return proceso;
}

t_pcb* deserializar_paquete_proceso_bloqueado(void* buffer) {
	int desplazamiento = 0;
	float tiempo_ejecutado;
	uint32_t ID_proceso;

	memcpy(&ID_proceso, buffer + desplazamiento, sizeof(uint32_t)); // id del proceso para buscarlo en la lista global
	desplazamiento += sizeof(uint32_t);

	t_pcb* proceso = get_proceso(ID_proceso);

	memcpy(&proceso->tiempo_bloqueado, buffer + desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	memcpy(&tiempo_ejecutado, buffer + desplazamiento, sizeof(float)); // milisegundos que ejecuto el proceso
	desplazamiento += sizeof(float);

	printf("\n\nTiempo que ejecuto el proceso %i: %f rafagas de cpu\n\n", proceso->ID, tiempo_ejecutado);

	memcpy(&proceso->program_counter, buffer + desplazamiento, sizeof(uint32_t)); // actualizamos el PC
	desplazamiento += sizeof(uint32_t);

	proceso->rafaga_real_anterior = tiempo_ejecutado + proceso->tiempo_acumulado;
	proceso->estimacion_anterior = proceso->estimacion_rafaga + proceso->tiempo_acumulado;
	float prox_rafaga_aux = estimar_proxima_rafaga(proceso);
	proceso->estimacion_rafaga = prox_rafaga_aux;

//	printf("\n\nRafaga real anterior del proceso %i: %f\n", proceso->ID, proceso->rafaga_real_anterior);
//	printf("\nEstimacion anterior del proceso %i: %f\n", proceso->ID, proceso->estimacion_anterior);
//	printf("\nProxima rafaga del proceso %i: %f rafagas de cpu\n\n", proceso->ID, proceso->estimacion_rafaga);

	return proceso;
}

t_pcb* deserializar_paquete_proceso_terminado(void* buffer) {
	int desplazamiento = 0;
	uint32_t ID_proceso;

	memcpy(&ID_proceso, buffer + desplazamiento, sizeof(uint32_t)); // id del proceso para buscarlo en la lista global
	desplazamiento += sizeof(uint32_t);

	return get_proceso(ID_proceso);
}
