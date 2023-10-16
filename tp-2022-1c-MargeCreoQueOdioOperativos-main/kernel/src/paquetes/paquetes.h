#ifndef CLIENTE_PAQUETES_H_
#define CLIENTE_PAQUETES_H_

#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/collections/list.h>

#include "../cliente/cliente.h"

// Variables

typedef enum {
	NUEVO_PROCESO = 1002, // Cuando consola nos pide ejecutar un nuevo proceso
	INICIAR_PROCESO = 1003, // Le enviamos a mm el pedido de una tabla de paginas para el proceso, enviar id + tamanio
	NUMERO_TABLA_PAGINAS = 1004, // Cuando mm nos manda el numero de la tabla de paginas
	PROCESO_A_EJECUTAR = 1005, // Cuando le enviamos el pcb a cpu para que lo ejecute
	SUSPENSION_PROCESO = 1006, // Cuando le avisamos a mm
	PROCESO_BLOQUEADO = 1007, // Cuando cpu nos avisa que un proceso se bloqueo
	PROCESO_TERMINADO = 1008, // Cuando cpu nos avisa que un proceso termino de ejecutar
	INTERRUPCION = 1009,
	LEVANTAR_PROCESO_SUSPENDIDO = 1010,
	PROCESO_SUSPENDIDO = 1011,
	PROCESO_DESALOJADO = 1012,
	ESTRUCTURAS_LIBERADAS = 1013
} op_code;

typedef struct {
	int32_t size;
	void* stream;
} t_buffer;

typedef struct {
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

typedef struct {
	char identificador[6];
	uint32_t parametro_1;
	uint32_t parametro_2;
} t_instruccion;

// Prototipo de funciones para crear paquetes a enviar a cpu y memoria.

t_paquete* paquete_pedido_tabla_de_paginas(uint32_t, uint32_t); // ID proceso + tamanio
t_paquete* paquete_levantar_proceso_suspendido(uint32_t);
t_paquete* paquete_aviso_interrupcion();
t_paquete* paquete_aviso_suspension_proceso(uint32_t); // le avisamos a mm que suspenda el proceso
t_paquete* paquete_aviso_fin_proceso_memoria(uint32_t); // le avisamos a mm que el proceso termino
t_paquete* paquete_aviso_fin_proceso_consola(); // le avisamos a consola que el proceso ya termino de ejecutar

// Deserializacion paquete nuevo proceso
void get_instrucciones(void*, t_list*);
uint32_t get_tamanio_proceso(void*);

// Prototipos de otras funciones

void* serializar_paquete(t_paquete*, int32_t);
void enviar_mensaje(char*, int);
void enviar_paquete(t_paquete*, int);
void eliminar_paquete(t_paquete*);

int recibir_operacion(int);
void* recibir_buffer(int*, int);

#endif /* CLIENTE_PAQUETES_H_ */
