/*
 * ciclo_instruccion.h
 *
 *  Created on: 31 may. 2022
 *      Author: utnso
 */

#ifndef CICLOINSTRUCCION_H_
#define CICLOINSTRUCCION_H_
#include "globals.h"
#include <math.h>
#include <stdlib.h>
#include "./cliente/armadorPaquete.h"

int buscar_operandos;
uint32_t operando;
int interrupcion_pendiente;
int hay_proceso_para_ejecutar;
struct timeval inicio_ejecucion, fin_ejecucion;
float tiempo_ejecucion;

t_dictionary* tlb;

void inicializar_variables();
void ejecutar_ciclo_instruccion();
t_instruccion fetch();
void decode(char* identificador);
void fetch_operands(t_instruccion instruccion); // la cpu tendra una variable que representara a los operandos (como si fuera un registro)
void execute(t_instruccion instruccion);
void check_interrupt();
uint32_t leer_de_memoria(uint32_t direccion_fisica);
uint32_t obtener_nro_pagina(uint32_t direccion_logica);
uint32_t buscar_operando_en_memoria(uint32_t direccion_fisica);
void desalojar_proceso();
void escribir_en_memoria(uint32_t direccion_logica, uint32_t valor);
void enviar_syscall_io_bloqueante_a_kernel(uint32_t tiempo_bloqueo);
void enviar_syscall_finalizacion_a_kernel();
uint32_t obtener_direccion_fisica(uint32_t direccion_logica);
void calcular_tiempo_ejecucion();
void finalizar_cuenta_tiempo_ejecucion();
bool pagina_esta_en_tlb(uint32_t numero_pagina);
bool es_algoritmo(char* algoritmo);
void inicializar_tlb();
void destruir_tlb();
uint32_t obtener_frame_de_tlb(uint32_t nro_pagina);
void agregar_entrada_a_tlb(uint32_t numero_pagina, uint32_t marco);
void quitar_entrada_tlb(uint32_t numero_pagina);
uint32_t obtener_frame_de_tabla_de_nivel_2(int socket_memoria, uint32_t tabla_nivel_1, uint32_t entrada_tabla_1er_nivel, uint32_t entrada_tabla_2do_nivel, uint32_t* hubo_reemplazo_en_memoria, uint32_t* pagina_reemplazada_en_memoria);
void vaciar_tlb();
bool tlb_vacia();
bool tlb_llena();
void actualizar_cola_de_algoritmo_por_referencia_a_una_pagina_en_tlb(uint32_t numero_pagina);
void actualizar_cola_de_algoritmo_por_reemplazo_hecho(uint32_t pagina_victima, uint32_t pagina_nueva);
void actualizar_cola_de_algoritmo_para_dejar_tlb_consistente_por_reemplazo_hecho_en_memoria(uint32_t pagina_victima, uint32_t pagina_nueva);
void actualizar_cola_de_algoritmo_cuando_la_tlb_no_esta_llena(uint32_t pagina_nueva);
uint32_t elegir_victima_segun_algoritmo();
void actualizar_cola_de_algoritmo();
uint32_t elegir_victima_segun_fifo();
uint32_t elegir_victima_segun_lru();
uint32_t validar_direccion_fisica(int socket_memoria, uint32_t pid, uint32_t numero_pagina, uint32_t desplazamiento);

// TODO:
bool proxima_instruccion(char* identificador);

#endif /* CICLOINSTRUCCION_H_ */
