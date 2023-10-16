#ifndef MEMORIA_H_
#define MEMORIA_H_
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<signal.h>
#include<unistd.h>
#include<netdb.h>
#include <commons/string.h>

#include "globals.h"
#include "swap.h"

/*(id,tamanio)*/
uint32_t iniciar_proceso_en_memoria(uint32_t,uint32_t);
/**/
void suspender_proceso_en_memoria(uint32_t proceso_id);
/**/
uint32_t escribir_en_memoria(uint32_t proceso_id,uint32_t pagina,uint32_t dir_fisica,void*stream);


/*
 * @desc:recibe la cantidad de paginas de un proceso para calcular la cantidad de tablas de nvl 1 correspondientes
 * */
uint32_t tabla_lvl1_totales(uint32_t);


uint32_t retorntar_nro_tabla_nvl2(uint32_t tabla,uint32_t entrada);



uint32_t es_direccion_valida(uint32_t proceso_id,uint32_t pagina,uint32_t offset);

/*@desc: cambia el bit de presencia a 0 de un proceso dado*/
void cambiar_bit_presencia_a_0_del_proceso(uint32_t);
void cambiar_bit_uso_a_0_del_proceso(uint32_t);
void cambiar_bit_modificado_a_0_del_proceso(uint32_t);

/*Recorre el proceso y cambia todos sus valores*/
void suspender_de_tabla(uint32_t proceso_id);
void escribir_en_eu(uint32_t dir_fisica, uint32_t valor);
uint32_t leer_en_eu(uint32_t dir_fisica);

void eliminar_proceso_de_eu(uint32_t proceso_id);

void marcos_disponibles(Tabla_nvl1* tabla1,uint32_t pagina,Entrada_tabla_nvl2* nodo_entrada_2);
void clock_n(uint32_t proceso_id,uint32_t pagina);
void clock_m(uint32_t proceso_id,uint32_t pagina);

void cambiar_bit_m(uint32_t id_proceso, uint32_t dir_fisica);
void modificar_tabla(uint32_t proceso_id, uint32_t pagina);
void finalizar_proceso(uint32_t proceso_id);
uint32_t remplazo_encotrado(Tabla_nvl1* tabla_1, Entrada_tabla_nvl2* entrada_2,	Remplazo_paginas* remplazo,uint32_t pagina_remplazo,uint32_t proceso_id,int j);


/*FUNCIONES DE TABLA DE PAGINAS*/

/**
 * @Desc: recibe la cantidad de tablas que usara mi proceso y retorna una tabla de nibel 1 donde calse
 * 		  mi proceso, caso contrario retorna una tabla vacia
 * */
Tabla_nvl1* tabla_nivel_1_a_usar(uint32_t cant_tablas_nvl2);

/**
 * @Desc: recibe la cantidad de paginas total que tendra mi proceso y en base a eso
 * retorna cuantas tablas de nivel dos va a usar mi proceso
 * */
uint32_t tablas_de_nvl2_a_usar(uint32_t cant_pagina_proceso);

/*
 * @desc: retorna como valor el marco actual del proceso
 * recibe como parametros el numero de tabla de nvl1 y el numero de tabla de nvl2
 */
uint32_t devolver_marco(uint32_t tabla_1,uint32_t entrada_1,uint32_t entrada_2);


/*
 * @desc:recibe el valor de un marco libre para devolverlo a la lista de marcos disponibles
 * liberar_marco(nro_marco)
 */



/*
 * @desc:retorna el valor de un marco disponible y lo libera de la lista
 * obtener_marco_disp()
 */
uint32_t obtener_marco_disp();

/*@desc: retorna la cantidad de marcos ocupados de un proceso*/
uint32_t marcos_ocupados(uint32_t);

void cambiar_bit_uso(uint32_t,uint32_t,uint32_t,uint32_t);
void cambiar_bit_presencia(uint32_t,uint32_t,uint32_t,uint32_t);
void cambiar_bit_modificado(uint32_t,uint32_t,uint32_t,uint32_t);
/*@desc: Retorna la tabla de lvl1 dado un proceso
 * */
Tabla_nvl1* buscar_proceso_tabla1(uint32_t proceso_id);
int tablas_max_proceso(uint32_t);

uint32_t remplazo_clock(uint32_t proceso_id,uint32_t pagina_a_remplazar);
/*
 * @desc: el proceso se encuentra en memoria?
 */
bool esta_en_memoria(uint32_t,uint32_t);
void* leer_pagina_eu(uint32_t marco);
void borrar_archivo_de_memoria(uint32_t proceso_id);
void escribir_pagina_eu(uint32_t dir_fisica, void* pagina);
void swapear_pagina(Tabla_nvl1* tabla_1,uint32_t nro_marco,uint32_t nro_pagina);
void traer_pagina_a_eu(Tabla_nvl1* tabla1,uint32_t nro_pagina,uint32_t nro_marco);
void swapear_proceso(uint32_t proceso_id);

#endif /* MEMORIA_H_ */
