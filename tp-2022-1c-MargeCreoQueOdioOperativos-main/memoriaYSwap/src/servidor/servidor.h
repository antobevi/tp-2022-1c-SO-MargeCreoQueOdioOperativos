/*
 * servidor.h
 *
 *  Created on: 5 abr. 2020
 *      Author: utnso
 */

#ifndef SERVIDOR_H_
#define SERVIDOR_H_

#include <stdio.h>
#include <stdlib.h>
#include "../globals.h"

#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <commons/log.h>
#include <commons/config.h>
#include <semaphore.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>

#include "../memoria.h"
//GLOBALES
t_log* logger;
t_config* config;
pthread_mutex_t mutex_log;
pthread_mutex_t sem_cliente;
bool server_levantado;



// PROTOTIPOS

uint32_t levantar_server_multithread();
void atender_cliente(int32_t);
uint32_t atender_suscripcion(int32_t);
uint32_t levantar_server_singlethread();
void leer_config_y_setear();
void atender_single_request(uint32_t );
bool patova(int32_t);

void* deserializar(uint32_t, void*, uint32_t );
op_code conseguir_codigo_operacion(int32_t);
void* recibir_paquete(uint32_t*, uint32_t);
void ejecutar_operacion(void*, op_code,int);


void obtener_id_y_tamanio(void*,uint32_t*,uint32_t*);
/*retorna un bubber con los valores necesiarios para mandarle a CPU*/
void* inicializar_cpu();

#endif /* SERVIDOR_H_ */
