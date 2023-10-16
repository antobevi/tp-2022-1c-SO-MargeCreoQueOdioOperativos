#include "servidor/servidor.h" // Ya incluye globals.h

int main(int argc,char** argv) {

	remove("log_global.log");

	char* nombre_config = string_duplicate(argv[1]);

	inicializar_loggers();
	inicializar_config(nombre_config);
	iniciar_estructuras_procesos();

	free(nombre_config);

//	socket_memoria = conectar_con_servidor(IP_MEMORIA, PUERTO_MEMORIA);
//	socket_cpu_dispatch = conectar_con_servidor(IP_CPU, PUERTO_CPU_DISPATCH);
//	socket_cpu_interrupt = conectar_con_servidor(IP_CPU, PUERTO_CPU_INTERRUPT);

	// Threads
	pthread_t thread_server;
	pthread_t thread_largo_plazo;
	pthread_t thread_mediano_plazo;
	pthread_t thread_corto_plazo;

	if(pthread_create(&thread_server, NULL, (void*)levantar_servidor, NULL)) {
		pthread_mutex_lock(&sem_log_global);
		log_error(log_global, "Error al crear el thread para el server.");
		pthread_mutex_unlock(&sem_log_global);
	}

	ejecutar_planificadores = 1; // Esta variable se usa en los whiles de las funciones de planificadores para que ejecuten

	if(pthread_create(&thread_largo_plazo, NULL, (void*)ejecutar_planificador_largo_plazo, NULL)) {
		pthread_mutex_lock(&sem_log_global);
		log_error(log_global, "Error al crear el thread para el planificador de largo plazo.");
		pthread_mutex_unlock(&sem_log_global);
	}

	if(pthread_create(&thread_corto_plazo, NULL, (void*)ejecutar_planificador_corto_plazo, NULL)) {
		pthread_mutex_lock(&sem_log_global);
		log_error(log_global, "Error al crear el thread para el planificador de corto plazo.");
		pthread_mutex_unlock(&sem_log_global);
	}

	if(pthread_create(&thread_mediano_plazo, NULL, (void*)ejecutar_planificador_mediano_plazo, NULL)) {
		pthread_mutex_lock(&sem_log_global);
		log_error(log_global, "Error al crear el thread para el planificador de mediano plazo.");
		pthread_mutex_unlock(&sem_log_global);
	}

	pthread_detach(thread_largo_plazo);
	pthread_detach(thread_corto_plazo);
    pthread_detach(thread_mediano_plazo);
	pthread_join(thread_server, 0);

	pthread_mutex_lock(&sem_log_global);
	log_info(log_global, "Fin del programa. Gracias, vuelva prontos!");
	pthread_mutex_unlock(&sem_log_global);

	destruir_estructuras_procesos();
	destruir_config();
	destruir_loggers();

	return EXIT_SUCCESS;
}

// ----------------------------------------------- Funciones de inicializacion ----------------------------------------------- //

void inicializar_config(char* nombre_config) {

	char* RUTA_CONFIG = malloc(strlen("../Configs/")+1);

	strcpy(RUTA_CONFIG, "../Configs/");
	string_append(&RUTA_CONFIG, nombre_config);

	config = config_create(RUTA_CONFIG);

	free(RUTA_CONFIG);

	char* mem_ip = config_get_string_value(config, "IP_MEMORIA");
	IP_MEMORIA = malloc(strlen(mem_ip)+1);
	strcpy(IP_MEMORIA, mem_ip);

	char* cpu_ip = config_get_string_value(config, "IP_CPU");
	IP_CPU = malloc(strlen(cpu_ip)+1);
	strcpy(IP_CPU, cpu_ip);

	char* planif_alg = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
	ALGORITMO_PLANIFICACION = malloc(strlen(planif_alg)+1);
	strcpy(ALGORITMO_PLANIFICACION, planif_alg);

	PUERTO_MEMORIA = config_get_int_value(config,"PUERTO_MEMORIA");
	PUERTO_CPU_DISPATCH = config_get_int_value(config,"PUERTO_CPU_DISPATCH");
	PUERTO_CPU_INTERRUPT = config_get_int_value(config,"PUERTO_CPU_INTERRUPT");
	PUERTO_ESCUCHA = config_get_int_value(config,"PUERTO_ESCUCHA");
	ESTIMACION_INICIAL = config_get_double_value(config,"ESTIMACION_INICIAL");
	ALFA = config_get_double_value(config,"ALFA");
	GRADO_MULTIPROGRAMACION = config_get_int_value(config,"GRADO_MULTIPROGRAMACION");
	//printf("\n\nGRADO_MULTIPROGRAMACION: %i\n\n", GRADO_MULTIPROGRAMACION);
	TIEMPO_MAXIMO_BLOQUEADO = config_get_int_value(config,"TIEMPO_MAXIMO_BLOQUEADO");
	/*
	pthread_mutex_lock(&sem_log_global);
	log_info(log_global, "> CONFIGURACIONES KERNEL <");
	log_info(log_global, "Algoritmo de planificacion: %s", ALGORITMO_PLANIFICACION);
	log_info(log_global, "Grado de multiprogramacion: %i", GRADO_MULTIPROGRAMACION);
	log_info(log_global, "Estimacion inicial: %f", ESTIMACION_INICIAL);
	log_info(log_global, "Alfa: %f", ALFA);
	log_info(log_global, "Tiempo maximo bloqueado: %f", TIEMPO_MAXIMO_BLOQUEADO);
	pthread_mutex_unlock(&sem_log_global);
	*/
}

void inicializar_loggers() {
	log_global = log_create("kernel.log", "LOG-GLOBAL-KERNEL", 1, LOG_LEVEL_INFO);

	if(pthread_mutex_init(&sem_log_global, NULL) != 0) {
		log_error(log_global, "Error creando el mutex para el log global de Kernel.");
	}
}

void destruir_config() {
	free(IP_MEMORIA);
	free(IP_CPU);
	free(ALGORITMO_PLANIFICACION);

	config_destroy(config);
}

void destruir_loggers() {
	pthread_mutex_destroy(&sem_log_global);
	log_destroy(log_global);
}
