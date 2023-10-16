#ifndef CLIENTE_PAQUETES_H_
#define CLIENTE_PAQUETES_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <commons/log.h>
#include <commons/collections/list.h>

// Variables

typedef struct {
	char identificador[6]; // El ayu recomendo usar char* en lugar de un vector
	uint32_t parametro_1;
	uint32_t parametro_2;
} t_instruccion;

typedef enum {
	NUEVO_PROCESO = 1002,
	PROCESO_TERMINADO = 1008
} op_code;

typedef struct {
	int32_t size;
	void* stream;
} t_buffer;

typedef struct {
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

// Prototipo de funciones

t_paquete* paquete_nuevo_proceso(t_list*, uint32_t);
void deserializar_paquete_fin_proceso(t_paquete*); // ver que debe retornar, un codigo?

void* serializar_paquete(t_paquete*, int32_t);
void enviar_paquete(t_paquete*, int);
void eliminar_paquete(t_paquete*);

#endif /* CLIENTE_PAQUETES_H_ */
