#include "consola.h"

// Esta implementación es para tener múltiples consolas. Ejemplo de uso en la terminal: ./consola 5 home/utnso/archivo.dat

int main(int argc, char** argv) {

	iniciar_logger();
	iniciar_config();

	if(argc > 3) {
		log_error(log_consola, "Argumentos invalidos. La estructura debe ser <nombrePrograma> <tamanio> <path>");

		return EXIT_FAILURE;
	}

	else {
		char* tamanio = string_duplicate(argv[1]); // Esta funcion hace un malloc, el atoi no puede usarse sin antes reservar mm
		char* path = argv[2];

		uint32_t tamanio_proceso = atoi(tamanio);

		if(tamanio_proceso == 0) {
			log_error(log_consola, "Debe ingresarse un tamanio valido.");
		}

		else {
			FILE* archivo = fopen(path, "r");

			if(archivo == NULL) {
				log_error(log_consola, "Error al intentar abrir el archivo.");

				return EXIT_FAILURE;
			}

			else {
				instanciar_consola(tamanio_proceso, archivo);
				fclose(archivo);
			}
		}

		free(tamanio);
	}

	liberar_memoria();

	return 0;
}

//Funciones de inicializacion y liberacion de memoria

void iniciar_logger() {
	log_consola = log_create("consola.log", "LOG_CONSOLA", 1, LOG_LEVEL_INFO);

	// Inicializo el semaforo para el log global y asi sincronizar. Ver si es necesario en este modulo!
	pthread_mutex_init(&semaforo_log, NULL);

}

void iniciar_config() {
	config = config_create("../consola.cfg"); // consola.c esta dentro de src y consola.cfg esta fuera

	char* kernel_ip = config_get_string_value(config, "IP_KERNEL"); // no hacer un free de este string!
	IP_KERNEL = malloc(strlen(kernel_ip)+1);
	strcpy(IP_KERNEL, kernel_ip);

	PUERTO_KERNEL = config_get_int_value(config, "PUERTO_KERNEL");
}

void liberar_memoria() {
	log_destroy(log_consola);
	pthread_mutex_destroy(&semaforo_log);
	free(IP_KERNEL);
	config_destroy(config);
}

// Funciones auxiliares

void instanciar_consola(uint32_t tamanio_proceso, FILE* archivo) {
	t_list* lista_instrucciones = list_create();

	while(!feof(archivo)) {
		char* instruccion_leida;

		instruccion_leida = leer_instruccion(archivo);
		t_instruccion* instruccion = crear_instruccion(instruccion_leida);
		mostrar_instruccion(instruccion);

		list_add(lista_instrucciones, instruccion);

		free(instruccion_leida);
	}

	t_paquete* paquete = paquete_nuevo_proceso(lista_instrucciones, tamanio_proceso);
	int socket_kernel = conectar_con_servidor(IP_KERNEL, PUERTO_KERNEL);
	enviar_paquete(paquete, socket_kernel);
	eliminar_paquete(paquete);

	int32_t confirmacion = 0;

	if(recv(socket_kernel, &confirmacion, sizeof(int32_t), 0)) {
		if(confirmacion == 1008) { // PROCESO_TERMINADO
			pthread_mutex_lock(&semaforo_log);
			log_info(log_consola, "El proceso fue ejecutado exitosamente.");
			pthread_mutex_unlock(&semaforo_log);
		}
	} else {
		log_error(log_consola, "Error al ejecutar el proceso.");
	}

	list_destroy_and_destroy_elements(lista_instrucciones, (t_instruccion*)free_nodo);
}

// Funciones auxiliares

void free_nodo(t_instruccion* instruccion) {
	free(instruccion);
}

void free_doble_puntero(char** linea_spliteada) {
	int i = 0;

	while(linea_spliteada[i] != NULL){
		free(linea_spliteada[i]);
		i++;
	}

	free(linea_spliteada);
}

int linea_spliteada_valida(char** linea_spliteada) {

	int longitud = longitud_linea_spliteada(linea_spliteada);

	return string_equals_ignore_case(linea_spliteada[0], "instanciar") && longitud == 3 && strcmp(linea_spliteada[1], "") && strcmp(linea_spliteada[2], "");
}

int longitud_linea_spliteada(char** linea_spliteada) {
	int i = 0;

	while(linea_spliteada[i] != NULL) {
		i++;
	}

	return i;
}

int fin_de_programa(char** linea_spliteada) {

	return string_equals_ignore_case(linea_spliteada[0], "fin");
}

// Funciones instrucciones

char* leer_instruccion(FILE* archivo){
	char* linea_leida = malloc(sizeof(char) * 20);

	// Se hardcodea con 20, ninguna instrucción (línea) tendrá más de 20 caracteres
	fgets(linea_leida, 20, archivo);
	char** linea_leida_sin_salto_de_linea = string_split(linea_leida, "\n");

	memcpy(linea_leida, linea_leida_sin_salto_de_linea[0], strlen(linea_leida_sin_salto_de_linea[0]) + 1);

	free_doble_puntero(linea_leida_sin_salto_de_linea);

	return linea_leida;
}

t_instruccion* crear_instruccion(char* instruccion_leida) {
	char** partes_instruccion = string_split(instruccion_leida, " ");
	t_instruccion* instruccion = nueva_instruccion();

	inicializar_instruccion(instruccion, partes_instruccion);

	free_doble_puntero(partes_instruccion);

	return instruccion;
}

t_instruccion* nueva_instruccion() {
	t_instruccion* instruccion = malloc(sizeof(t_instruccion));

	//Esto se hace porque si no Valgrind da error por no inicializar los parámetros
	instruccion->parametro_1 = 0;
	instruccion->parametro_2 = 0;

	return instruccion;
}

void inicializar_instruccion(t_instruccion *instruccion, char** partes_instruccion) {
	char identificador[6];

	memcpy(identificador, partes_instruccion[0], strlen(partes_instruccion[0])+1);
	memcpy(instruccion -> identificador, identificador, strlen(identificador) + 1);

	/*NO_OP 5
	I/O 3000
	READ 0
	WRITE 4 42
	COPY 0 4
	EXIT*/

	if(string_equals_ignore_case("NO_OP", identificador) || string_equals_ignore_case("I/O", identificador) || string_equals_ignore_case("READ", identificador)) {
		instruccion -> parametro_1 = atoi(partes_instruccion[1]);
	}

	if(string_equals_ignore_case("WRITE", identificador) || string_equals_ignore_case("COPY", identificador)) {
		instruccion -> parametro_1 = atoi(partes_instruccion[1]);
		instruccion -> parametro_2 = atoi(partes_instruccion[2]);
	}

	// Si es EXIT, los parámetros ya están inicializados en 0
}

void mostrar_instruccion(t_instruccion* instruccion) {
	printf("El identificador es: %s \n", instruccion->identificador);

	printf("El primer parametro es: %d \n", instruccion->parametro_1);

	printf("El segundo parametro es: %d \n", instruccion->parametro_2);
}
