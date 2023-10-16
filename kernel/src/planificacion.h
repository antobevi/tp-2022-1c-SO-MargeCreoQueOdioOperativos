#ifndef PLANIFICACION_H_
#define PLANIFICACION_H_

#include "globals.h"
#include "paquetes/paquetes.h"

// Variables

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
	uint32_t ID;
	uint32_t tamanio;
	estado estado_actual;
	t_list* instrucciones;
	uint32_t program_counter;
	uint32_t tabla_paginas;
	float estimacion_rafaga; // Proxima rafaga a ejecutar
	float rafaga_real_anterior; // Lo que ejecuto posta antes
	float estimacion_anterior; // Lo que se estimo antes
	uint32_t tiempo_bloqueado;
	float tiempo_acumulado; // Si el proceso es desalojado, nos guardamos lo que ejecuto
	pthread_t thread_control_suspension; // Hilo que controla el tiempo que cada proceso pasa bloqueado
	int socket_consola; // Para avisarle cuando finalice (usando la misma conexion inicial)
} t_pcb;

uint32_t ids_procesos; // esta variable se va incrementando dsp de asignar con cada proceso nuevo que se crea
t_list* procesos; // lista de todos los procesos del sistema
int hay_proceso_ejecutando; // cuando enviamos un proceso a cpu esta var vale 1 para asi indicar que se puede lanzar una interrupcion

// Colas de estados

t_list* cola_new;
t_list* cola_ready;
t_list* cola_execute;
t_list* cola_suspended_ready;
t_list* cola_blocked;
t_list* cola_exit;

// Semaforos

pthread_mutex_t sem_ids_procesos; // Para sincronizar la variable ids_procesos
pthread_mutex_t sem_lista_procesos;
pthread_mutex_t sem_hay_proceso_ejecutando;

//TODO: semaforos para probar sincronizaciones
pthread_mutex_t sem_conexiones;
pthread_mutex_t sem_estado_proceso_blocked;

pthread_mutex_t sem_cola_new;
pthread_mutex_t sem_cola_ready;
pthread_mutex_t sem_cola_exec;
pthread_mutex_t sem_cola_susp_ready;
pthread_mutex_t sem_cola_blocked;
pthread_mutex_t sem_cola_exit;

sem_t* sem_nuevo_proceso; // semaforo para evitar la espera activa en el planificador de largo plazo
sem_t* sem_grado_multiprogramacion;
sem_t* sem_hay_procesos_ready;
sem_t* sem_hay_proceso_blocked;

// Prototipos de funciones

void iniciar_estructuras_procesos();
void iniciar_colecciones();
void iniciar_semaforos();

void destruir_estructuras_procesos();
void destruir_colecciones();
void destruir_semaforos();
void destruir_proceso(t_pcb*); // cuando un proceso termina

void crear_proceso(t_list*, uint32_t, int); // recibe la lista de instrucciones, el tamanio y el socket de consola para responderle dsp
void ejecutar_planificador_largo_plazo();
void ejecutar_planificador_mediano_plazo();
void ejecutar_planificador_corto_plazo();

// Prototipos de funciones auxiliares

float estimar_proxima_rafaga(t_pcb*);
t_pcb* menor_proxima_rafaga(t_pcb*, t_pcb*);
void cambiar_estado_proceso(t_pcb*, estado); // Funcion que cambia de estado y de cola al proceso segun se indique
void interrumpir_ciclo_de_instruccion();
void inicializar_lista_instrucciones(t_pcb*, t_list*);
void verificar_tiempo_de_bloqueo(t_pcb*); // funcion del hilo thread_control_suspencion
t_pcb* proximo_proceso_a_ejecutar();
t_pcb* get_proceso(uint32_t);

// Prototipo de funciones para el armado de paquetes (definidos aca por la estructura pcb)

t_paquete* paquete_proceso_a_ejecutar(t_pcb*);

// Prototipo de funciones para deserializar paquetes enviados por consola, memoria y cpu (definidos aca por estructuras/funciones de procesos)

t_pcb* deserializar_paquete_proceso_desalojado(void*);
t_pcb* deserializar_paquete_proceso_bloqueado(void*);
t_pcb* deserializar_paquete_proceso_terminado(void*);

#endif /* PLANIFICACION_H_ */
