#include "../cliente/armadorPaquete.h"

t_paquete* armar_paquete_validacion_direccion_fisica(uint32_t pid, uint32_t numero_pagina,uint32_t desplazamiento) {
	t_buffer* buffer = malloc(sizeof(t_buffer));
	t_paquete* paquete = malloc(sizeof(t_paquete));

	uint32_t offset = 0;

	buffer->size = sizeof(uint32_t)*3;
	void* stream = malloc(buffer->size);

	memcpy(stream + offset, &(pid),sizeof(uint32_t));
	offset = offset + sizeof(uint32_t);
	memcpy(stream + offset, &(numero_pagina),sizeof(uint32_t));
	offset = offset + sizeof(uint32_t);
	memcpy(stream + offset, &(desplazamiento),sizeof(uint32_t));



	buffer->stream = stream;
	paquete->buffer = buffer;
	paquete->codigo_operacion = ENVIO_DATOS_TABLA_NIVEL_1;

	return paquete;
}


t_paquete* armar_paquete_lectura(uint32_t direccion_fisica, uint32_t pid){
	t_buffer* buffer = malloc(sizeof(t_buffer));
	t_paquete* paquete = malloc(sizeof(t_paquete));

	uint32_t offset = 0;

	buffer->size = sizeof(uint32_t)*2;
	void* stream = malloc(buffer->size);

	memcpy(stream + offset, &(direccion_fisica), sizeof(uint32_t));
	offset = offset + sizeof(uint32_t);
	memcpy(stream + offset, &(pid), sizeof(uint32_t));
	offset = offset + sizeof(uint32_t);

	buffer->stream = stream;
	paquete->buffer = buffer;
	paquete->codigo_operacion = LECTURA_MEMORIA;

	return paquete;
}

t_paquete* armar_paquete_escritura(uint32_t direccion_fisica, uint32_t valor_a_escribir, uint32_t pid){
	t_buffer* buffer = malloc(sizeof(t_buffer));
	t_paquete* paquete = malloc(sizeof(t_paquete));

	uint32_t offset = 0;

	buffer->size = sizeof(uint32_t)*3;
	void* stream = malloc(buffer->size);

	memcpy(stream + offset, &(direccion_fisica),sizeof(uint32_t));
	offset = offset + sizeof(uint32_t);
	memcpy(stream + offset, &(valor_a_escribir),sizeof(uint32_t));
	offset = offset + sizeof(uint32_t);
	memcpy(stream + offset, &(pid),sizeof(uint32_t));
	offset = offset + sizeof(uint32_t);

	buffer->stream = stream;
	paquete->buffer = buffer;
	paquete->codigo_operacion = ESCRITURA_MEMORIA;

	return paquete;
}

t_paquete* armar_paquete_syscall_io_bloqueante(uint32_t pid, uint32_t tiempo_bloqueo, float tiempo_ejecucion, uint32_t program_counter_actualizado) {
	t_paquete* paquete = malloc(sizeof(t_paquete));
	t_buffer* buffer = malloc(sizeof(t_buffer));
	int32_t offset = 0;

	buffer -> size = sizeof(uint32_t) + sizeof(uint32_t) + sizeof(float) + sizeof(uint32_t);

	void* stream = malloc(buffer->size);

	memcpy(stream + offset, &(pid), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &(tiempo_bloqueo), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &(tiempo_ejecucion), sizeof(float));
	offset += sizeof(float);
	memcpy(stream + offset, &(program_counter_actualizado), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	buffer->stream = stream;
	paquete->codigo_operacion = PROCESO_BLOQUEADO; // 1007
	paquete->buffer = buffer;
	return paquete;
}

t_paquete* armar_paquete_finalizacion_proceso(uint32_t pid) {
	t_paquete* paquete = malloc(sizeof(t_paquete));
	t_buffer* buffer = malloc(sizeof(t_buffer));
	int32_t offset = 0;

	buffer -> size = sizeof(uint32_t);

	void* stream = malloc(buffer->size);

	memcpy(stream + offset, &(pid), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	buffer->stream = stream;
	paquete->codigo_operacion = PROCESO_TERMINADO; // 1008
	paquete->buffer = buffer;
	return paquete;
}

t_paquete* armar_paquete_proceso_desalojado(uint32_t pid, float tiempo_ejecucion, uint32_t program_counter_actualizado) {
	t_paquete* paquete = malloc(sizeof(t_paquete));
	t_buffer* buffer = malloc(sizeof(t_buffer));
	int32_t offset = 0;

	printf("\n\nTIEMPO QUE EJECUTO: %f\n\n", tiempo_ejecucion);

	buffer -> size = sizeof(uint32_t) + sizeof(float) + sizeof(uint32_t);

	void* stream = malloc(buffer->size);

	memcpy(stream + offset, &(pid), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &(tiempo_ejecucion), sizeof(float));
	offset += sizeof(float);
	memcpy(stream + offset, &(program_counter_actualizado), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	buffer->stream = stream;
	paquete->codigo_operacion = PROCESO_DESALOJADO; // 1012
	paquete->buffer = buffer;
	return paquete;
}

void recibir_proceso_a_ejecutar_y_deserializar(void* buffer) {
	int32_t offset = 0;
	int32_t cantidad_instrucciones;

	pcb_global.instrucciones = list_create();

	memcpy(&(pcb_global.id), buffer + offset, sizeof(uint32_t));
	offset = offset + sizeof(uint32_t);
	memcpy(&(pcb_global.tamanio), buffer + offset, sizeof(uint32_t));
	offset = offset + sizeof(uint32_t);
	memcpy(&(pcb_global.estado_actual), buffer + offset, sizeof(estado));
	offset = offset + sizeof(estado);
	memcpy(&cantidad_instrucciones, buffer + offset, sizeof(int32_t));
	offset = offset + sizeof(int32_t);

	for(int i=0; i<cantidad_instrucciones; i++) {
		t_instruccion* instruccion = malloc(sizeof(t_instruccion));

		memcpy(&instruccion->identificador, buffer+offset, 6);
		offset += 6;
		memcpy(&instruccion->parametro_1, buffer+offset, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(&instruccion->parametro_2, buffer+offset, sizeof(uint32_t));
		offset += sizeof(uint32_t);

		list_add(pcb_global.instrucciones, instruccion);
	}

	memcpy(&(pcb_global.program_counter), buffer + offset, sizeof(uint32_t));
	offset = offset + sizeof(uint32_t);
	memcpy(&(pcb_global.tabla_paginas), buffer + offset, sizeof(uint32_t));
	offset = offset + sizeof(uint32_t);
	memcpy(&(pcb_global.estimacion_rafaga), buffer + offset, sizeof(float));
	offset = offset + sizeof(float);

}

//TODO: revisar
void acceso_a_tabla_2do_nivel(void* paquete,uint32_t nro_pagina, uint32_t tabla_nivel_2){



	memcpy(paquete, &(tabla_nivel_2),sizeof(uint32_t));

	memcpy(paquete+4, &(nro_pagina),sizeof(uint32_t));


}

void* serializar_paquete(t_paquete* paquete, int bytes){
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return magic;
}

void enviar_mensaje(char* mensaje, int socket_cliente, int cod_operacion){
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = cod_operacion;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}

void enviar_mensaje_inicial_memoria(int socket_servidor){
	int codigo = 1001;
	send(socket_servidor,&codigo,sizeof(int), 0);

}

void eliminar_paquete(t_paquete* paquete){
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

void enviar_paquete(t_paquete* paquete, int socket_cliente){
	int32_t bytes = paquete->buffer->size + 2*sizeof(int32_t);
	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
}

void liberar_conexion(int socket_cliente){
	close(socket_cliente);
}
