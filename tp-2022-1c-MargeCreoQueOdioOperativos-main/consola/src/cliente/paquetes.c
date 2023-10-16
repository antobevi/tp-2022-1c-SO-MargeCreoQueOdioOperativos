#include "paquetes.h"

t_paquete* paquete_nuevo_proceso(t_list* instrucciones, uint32_t tamanio_proceso) {
	t_paquete* paquete = malloc(sizeof(t_paquete));
	t_buffer* buffer = malloc(sizeof(t_buffer));
	int32_t offset = 0; // Desplazamiento

	buffer -> size = sizeof(uint32_t) + sizeof(int32_t); // tamanio proceso + cant. inst. = 4 + 4 = 8 bytes

	int contador = 0;
	uint32_t tamanio_instruccion;
	while(contador < list_size(instrucciones)) {
		t_instruccion* instruccion = list_get(instrucciones, contador);

		tamanio_instruccion = sizeof(instruccion->identificador) + sizeof(uint32_t) + sizeof(uint32_t); // 6 + 4 + 4 = 14 bytes
		buffer->size += tamanio_instruccion;

		contador++;
	}

	void* stream = malloc(buffer -> size); // 14 bytes*cant inst + 8 bytes => ej: 6 inst: 84 + 8 = 92 bytes total x paquete

	memcpy(stream + offset, &(tamanio_proceso), sizeof(uint32_t)); // 0 a 4 bytes
	offset += sizeof(uint32_t); // offset = 4 bytes
	memcpy(stream + offset, &(instrucciones -> elements_count), sizeof(int32_t)); // Para que KERNEL sepa cuantos elementos va a leer.
	offset += sizeof(int32_t); // 4 a 8 bytes, offset = 4 + 4 = 8

	contador = 0;
	while(contador < list_size(instrucciones)) {
		t_instruccion* instruccion = list_get(instrucciones, contador);

		memcpy(stream + offset, &(instruccion->identificador), sizeof(instruccion->identificador)); // arranca de 8
		offset += sizeof(instruccion->identificador);
		memcpy(stream + offset, &(instruccion->parametro_1), sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(stream + offset, &(instruccion->parametro_2), sizeof(uint32_t));
		offset += sizeof(uint32_t);

		contador++;
	}

	buffer -> stream = stream;
	paquete -> codigo_operacion = NUEVO_PROCESO;
	paquete -> buffer = buffer;

	return paquete;
}

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

void enviar_paquete(t_paquete* paquete_a_enviar, int socket_cliente) {
	int tamanio_paquete = paquete_a_enviar->buffer->size + sizeof(int32_t) + sizeof(op_code); // tamanio buffer + int del size + int del cod de op
	void* paquete_serializado = serializar_paquete(paquete_a_enviar, tamanio_paquete);

	send(socket_cliente, paquete_serializado, tamanio_paquete, 0);

	free(paquete_serializado);
}

void eliminar_paquete(t_paquete* paquete) {
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}
