#ifndef SERVIDOR_H_
#define SERVIDOR_H_

#include "../cicloInstruccion.h"

int socket_servidor_dispatch;
int socket_servidor_interrupt;
t_log* log_servidor;
bool server_levantado_dispatch;
bool server_levantado_interrupt;
int cod_op;

int levantar_servidor_en_puerto(int puerto);
int levantar_server_dispatch();
int levantar_server_interrupt();
bool validar(int32_t sock_cliente);
int recibir_operacion(int socket_cliente);
void* recibir_buffer(int* size, int socket_cliente);
void atender_cliente(int socket_cliente);
void ejecutar_operacion(void* buffer, int cod_op, int socket_cliente);
void iniciar_cuenta_tiempo_ejecucion();
pcb deserializar(void* buffer);
void destruir_instruccion(t_instruccion* instruccion);

#endif /* SERVIDOR_H_ */
