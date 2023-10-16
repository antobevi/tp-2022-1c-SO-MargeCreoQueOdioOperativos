#ifndef CONSOLA_H_
#define CONSOLA_H_

#include <semaphore.h>
#include <readline/readline.h>
#include <commons/string.h>
#include <commons/config.h>

#include "cliente/cliente.h"
#include "cliente/paquetes.h"

// Variables

t_config* config;
t_log* log_consola;
uint32_t PUERTO_KERNEL;
char* IP_KERNEL;
int fin_programa;
pthread_mutex_t semaforo_log;

// Prototipos de funciones

void iniciar_config();
void iniciar_logger();
void leer_consola(int*);
void instanciar_consola(uint32_t, FILE*);
void liberar_memoria();
int fin_de_programa(char**);
char* leer_instruccion(FILE*);
void obtener_instruccion(char*);
t_instruccion* crear_instruccion(char*);
t_instruccion* nueva_instruccion();
void mostrar_instruccion(t_instruccion*);
void inicializar_instruccion(t_instruccion*, char**);

void free_doble_puntero(char**);
int linea_spliteada_valida(char**);
int longitud_linea_spliteada(char**);
void free_nodo();

#endif /* CONSOLA_H_ */
