#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include <commons/collections/list.h>
#include "cliente.h"

typedef enum
{
	INTERRUPCION = 1009,
	PROCESO_A_EJECUTAR = 1005, // Cuando le enviamos el pcb a cpu para que lo ejecute
	INICIAR_CPU = 3000,
	ENVIO_DATOS_TABLA_NIVEL_1 = 3001,
	LECTURA_MEMORIA = 3002,
	ESCRITURA_MEMORIA = 3003,
	PROCESO_TERMINADO = 1008,
	PROCESO_BLOQUEADO = 1007,
	PROCESO_DESALOJADO = 1012,
	ESCRITURA_EXITOSA = 3020

}op_code;

typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

typedef enum {
	NEW = 1,
	READY = 2,
	EXEC = 3,
	BLOCKED = 4,
	SUSPENDED_BLOCKED = 5,
	SUSPENDED_READY = 6,
	EXIT = 7
} estado;

typedef struct {
 uint32_t id;
 uint32_t tamanio;
 estado estado_actual;
 t_list* instrucciones;
 uint32_t program_counter;
 int32_t tabla_paginas;
 float estimacion_rafaga;
} pcb;

typedef struct {
	char identificador[6]; // El ayu recomendo usar char* en lugar de un vector
	uint32_t parametro_1;
	uint32_t parametro_2;
} t_instruccion;

pcb pcb_global;

int conexion_dispatch; // socket para kernel (para enviar pcb actualizado)

int crear_conexion(char* ip, char* puerto);
void enviar_mensaje(char* mensaje, int socket_cliente, int cod_operacion);
t_paquete* crear_paquete(void);
t_paquete* crear_super_paquete(void);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void liberar_conexion(int socket_cliente);
void eliminar_paquete(t_paquete* paquete);
void enviar_mensaje_inicial_memoria(int);

t_paquete* paquete_tabla_entrada_nivel_1(uint32_t numero_tabla, uint32_t numero_entrada);
void acceso_a_tabla_2do_nivel(void* paquete,uint32_t nro_pagina, uint32_t tabla_nivel_2);
t_paquete* armar_paquete_escritura(uint32_t direccion_Fisica,uint32_t valor_a_escribir, uint32_t pid);
t_paquete* armar_paquete_lectura(uint32_t direccion_fisica, uint32_t pid);
t_paquete* armar_paquete_syscall_io_bloqueante(uint32_t pid, uint32_t tiempo_bloqueo, float tiempo_ejecucion, uint32_t program_counter_actualizado);
t_paquete* armar_paquete_proceso_desalojado(uint32_t pid, float tiempo_ejecucion, uint32_t program_counter_actualizado);
t_paquete* armar_paquete_finalizacion_proceso(uint32_t pid);
t_paquete* armar_paquete_validacion_direccion_fisica(uint32_t pid, uint32_t numero_pagina, uint32_t desplazamiento);
void recibir_proceso_a_ejecutar_y_deserializar(void* buffer);
