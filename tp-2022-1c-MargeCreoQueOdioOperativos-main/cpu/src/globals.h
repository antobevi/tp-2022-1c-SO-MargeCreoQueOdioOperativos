/*
 * globals.h
 *
 *  Created on: 31 may. 2022
 *      Author: utnso
 */

#ifndef GLOBALS_H_
#define GLOBALS_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdbool.h>
#include <string.h>
#include "pthread.h"
#include <sys/time.h>

uint32_t ENTRADAS_TLB;
char* REEMPLAZO_TLB;
uint32_t RETARDO_NOOP;
char* IP_MEMORIA;
uint32_t PUERTO_MEMORIA;
int PUERTO_ESCUCHA_DISPATCH;
int PUERTO_ESCUCHA_INTERRUPT;
uint32_t ENTRADAS_POR_TABLA_MEMORIA;
uint32_t TAM_PAGINA;
t_config* config;
t_log* log_global;
t_log* log_test;
t_log* log_usando;
t_queue* cola_fifo;
t_list* cola_lru;
uint32_t resultado_validacion;
pthread_mutex_t sem_log_global;
pthread_mutex_t sem_interrupcion;

void inicializar_loggers();
void destruir_loggers();
void inicializar_config(char* nombre_config);
void destruir_config();
void incializar_cola_para_algoritmo();
void destruir_cola_para_algoritmo();
bool es_algoritmo(char* algoritmo);


#endif /* GLOBALS_H_ */
