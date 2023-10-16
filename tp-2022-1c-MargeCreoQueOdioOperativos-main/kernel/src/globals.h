#ifndef GLOBALS_H_
#define GLOBALS_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>
#include <readline/readline.h>
#include <string.h>
#include <commons/string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>

// Variables para el config

t_config* config;
char* IP_MEMORIA;
uint32_t PUERTO_MEMORIA;
char* IP_CPU;
uint32_t PUERTO_CPU_DISPATCH;
uint32_t PUERTO_CPU_INTERRUPT;
uint32_t PUERTO_ESCUCHA;
char* ALGORITMO_PLANIFICACION;
float ESTIMACION_INICIAL;
float ALFA;
uint32_t GRADO_MULTIPROGRAMACION;
uint32_t TIEMPO_MAXIMO_BLOQUEADO;

// Variables globales

int ejecutar_planificadores;

int socket_memoria;
int socket_cpu_dispatch;
int socket_cpu_interrupt;

// Loggers

t_log* log_global;

// Semaforos

pthread_mutex_t sem_log_global;

// Funciones globales

void inicializar_config();
void inicializar_loggers();
void destruir_config();
void destruir_loggers();

#endif /* GLOBALS_H_ */
