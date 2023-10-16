#include "paquetes.h"

// --------------------------------------------------------- Paquetes --------------------------------------------------------- //

// Paquetes a enviar a Memoria Y Swap

t_paquete* paquete_pedido_tabla_de_paginas(uint32_t ID_proceso, uint32_t tamanio_proceso) {
	t_buffer* buffer = malloc(sizeof(t_buffer));
	t_paquete* paquete = malloc(sizeof(t_paquete));
	uint32_t offset = 0; // Desplazamiento

	buffer->size = sizeof(uint32_t) + sizeof(uint32_t);

	void* stream = malloc(buffer->size);

	memcpy(stream + offset, &(ID_proceso), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &(tamanio_proceso), sizeof(uint32_t));
	offset += sizeof(uint32_t);


	buffer->stream = stream;
	paquete->codigo_operacion = INICIAR_PROCESO;
	paquete->buffer = buffer;

	return paquete;
}

t_paquete* paquete_aviso_suspension_proceso(uint32_t ID_proceso) {
	t_buffer* buffer = malloc(sizeof(t_buffer));
	t_paquete* paquete = malloc(sizeof(t_paquete));
	uint32_t offset = 0; // Desplazamiento

	buffer->size = sizeof(uint32_t);

	void* stream = malloc(buffer->size);

	memcpy(stream + offset, &(ID_proceso), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	buffer->stream = stream;
	paquete->codigo_operacion = SUSPENSION_PROCESO;
	paquete->buffer = buffer;

	return paquete;
}

t_paquete* paquete_levantar_proceso_suspendido(uint32_t ID_proceso) {
	t_buffer* buffer = malloc(sizeof(t_buffer));
	t_paquete* paquete = malloc(sizeof(t_paquete));
	uint32_t offset = 0; // Desplazamiento

	buffer->size = sizeof(uint32_t);

	void* stream = malloc(buffer->size);

	memcpy(stream + offset, &(ID_proceso), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	buffer->stream = stream;
	paquete->codigo_operacion = LEVANTAR_PROCESO_SUSPENDIDO;
	paquete->buffer = buffer;

	return paquete;
}

t_paquete* paquete_aviso_fin_proceso_memoria(uint32_t ID_proceso) {
	t_buffer* buffer = malloc(sizeof(t_buffer));
	t_paquete* paquete = malloc(sizeof(t_paquete));
	uint32_t offset = 0; // Desplazamiento

	buffer->size = sizeof(uint32_t);

	void* stream = malloc(buffer->size);

	memcpy(stream + offset, &(ID_proceso), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	buffer->stream = stream;
	paquete->codigo_operacion = PROCESO_TERMINADO;
	paquete->buffer = buffer;

	return paquete;
}

// Paquetes a enviar a CPU

t_paquete* paquete_aviso_interrupcion() {
	t_buffer* buffer = malloc(sizeof(t_buffer));
	t_paquete* paquete = malloc(sizeof(t_paquete));

	buffer->size = 0;

	buffer->stream = NULL;
	paquete->codigo_operacion = INTERRUPCION;
	paquete->buffer = buffer;

	return paquete;
}

// Paquetes a enviar a Consola

t_paquete* paquete_aviso_fin_proceso_consola() {
	t_buffer* buffer = malloc(sizeof(t_buffer));
	t_paquete* paquete = malloc(sizeof(t_paquete));

	buffer->size = 0;

	buffer->stream = NULL;
	paquete->codigo_operacion = PROCESO_TERMINADO;
	paquete->buffer = buffer;

	return paquete;
}

// --------------------------------------------------------- Enviar --------------------------------------------------------- //

void enviar_mensaje(char* mensaje, int socket_cliente) {
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete -> codigo_operacion = 1001; // MENSAJE
	paquete -> buffer = malloc(sizeof(t_buffer));
	paquete -> buffer -> size = strlen(mensaje) + 1;
	paquete -> buffer -> stream = malloc(paquete->buffer->size);
	memcpy(paquete -> buffer -> stream, mensaje, paquete->buffer->size);

	int32_t bytes = paquete -> buffer -> size + 2*sizeof(int);

	void* paquete_a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, paquete_a_enviar, bytes, 0);

	eliminar_paquete(paquete);
	free(paquete_a_enviar);
}

void eliminar_paquete(t_paquete* paquete) {
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

void enviar_paquete(t_paquete* paquete_a_enviar, int socket_cliente) {
	int tamanio_paquete = paquete_a_enviar->buffer->size + sizeof(int32_t) + sizeof(op_code); // tamanio buffer + int del size + int del cod de op
	void* paquete_serializado = serializar_paquete(paquete_a_enviar, tamanio_paquete);

	send(socket_cliente, paquete_serializado, tamanio_paquete, 0);

	free(paquete_serializado);
}

void* recibir_buffer(int* size, int socket_cliente) {
	void* buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

// ------------------------------------------------------- Serializacion ------------------------------------------------------- //

void* serializar_paquete(t_paquete* paquete, int32_t bytes) {
	void* magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(op_code));
	desplazamiento += sizeof(op_code);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int32_t));
	desplazamiento += sizeof(int32_t);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento += paquete->buffer->size;

	return magic;
}

// ----------------------------------------------------- Deserializacion ----------------------------------------------------- //

int recibir_operacion(int socket_cliente) {
	int cod_op;

	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0) {
		return cod_op;
	} else {
		close(socket_cliente);
		return -1;
	}
}

uint32_t get_tamanio_proceso(void* buffer) {
	uint32_t tamanio_proceso = 0;
	int desplazamiento = 0;

	memcpy(&tamanio_proceso, buffer + desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	return tamanio_proceso;
}

void get_instrucciones(void* buffer, t_list* instrucciones) {
	int desplazamiento = sizeof(uint32_t); // El tamanio del proceso esta primero
	int cant_instrucciones = 0;

	memcpy(&cant_instrucciones, buffer + desplazamiento, sizeof(int32_t));
	desplazamiento += sizeof(int32_t);

	for(int i=0; i<cant_instrucciones; i++) {
		t_instruccion* instruccion = malloc(sizeof(t_instruccion));

		memcpy(&instruccion->identificador, buffer+desplazamiento, 6);
		desplazamiento += 6;
		memcpy(&instruccion->parametro_1, buffer+desplazamiento, sizeof(uint32_t));
		desplazamiento += sizeof(uint32_t);
		memcpy(&instruccion->parametro_2, buffer+desplazamiento, sizeof(uint32_t));
		desplazamiento += sizeof(uint32_t);

		list_add(instrucciones, instruccion);
	}
}
