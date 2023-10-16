
#ifndef GLOBALS_H_
#define GLOBALS_H_


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <sys/stat.h>
#include <commons/log.h>
#include <pthread.h>


#define VACIO 100000
/*-Tengo todas las tablas de mi proceso en la misma tabla o estan distribuidas*/
//Variables globales
char* IP;
uint32_t PUERTO;
t_config* config;
uint32_t TAM_MEMORIA;//ej:4096
uint32_t TAM_PAGINA; //64
uint32_t ENTRADAS_POR_TABLA; //4
uint32_t RETARDO_MEMORIA; // 1000
char* ALGORITMO_REEMPLAZO;// CLOCK-M o CLOCK
uint32_t MARCOS_POR_PROCESO;// 4
uint32_t RETARDO_SWAP;//2000
char* PATH_SWAP;// /home/utnso/swap
//DEFINIR CODIGOS DE OPERACION

int cg_tabla_nvl2;

typedef enum {
	NO_REEMPLAZO = 0,
	REEMPLAZO = 1,
    INICIAR_PROCESO = 1003,
    SUSPENSION_PROCESO = 1006,
    PROCESO_TERMINADO = 1008,
	HANDSHAKE = 3000,
	PEDIDO_MARCO = 3001,
	LECTURA_MEMORIA = 3002,
	ESCRITURA_MEMORIA = 3003
} op_code;

 uint32_t hubo_remplazo;
 uint32_t pagina_reemplazada;

//Structuras de las tablas correspondientes

typedef struct{
	uint32_t nro_marco;
	uint32_t nro_pagina;
	uint32_t bit_uso;
	uint32_t bit_presencia;
	uint32_t bit_modificado;
}Entrada_tabla_nvl2;

typedef struct{//TODO: Me conviene tener que procesos tengo apuntando a mi tabla de lvl2?
	uint32_t nro_tabla_nvl2;
	t_list* entradas_tabla_nvl2;
}Entrada_tabla_nvl1;

typedef struct{
	uint32_t pag_en_memoria;
}Remplazo_paginas;

typedef struct{
	uint32_t nro_tabla_nvl1;
	uint32_t proceso_id;
	t_list* Entradas_tabla_nvl1;
	t_list* lista_aux;
	uint32_t puntero;
	char* archivo_swap;
	uint32_t tam_archivo;
	uint32_t tam_proceso;
//	uint32_t pags_en_memori[50];

}Tabla_nvl1;


typedef struct{
	uint32_t nro_marco_disp;
}Marco;

t_list* tabla_de_paginas;
t_list* marcos_disp;

t_log* log_global;
t_log* log_tester;
t_log* servidor;
void* espacio_usuario;

void inicializar_config(char* nombre_config);
void liberar_memoria();
void borrar_logs(char* nombre);


//NODOS
/*
* @Desc: crea una fila o columna de nivel dos
*/
Entrada_tabla_nvl1* instanciar_entrada_nvl1();

/* @Descripcion: retorna una fila de la tabla de paginas de nivel dos
 * @Parametros: instanciar_nodo_lvl2(nro_marco,nro_pagina,bit_uso,,bit_presencia,bit_modificado,puntero;)*/
Entrada_tabla_nvl2* instanciar_entradas_nvl2(uint32_t nro_pagina);
/*
 *Instancia una tabla de nivel 1
 */
Tabla_nvl1* instanciar_tabla_nvl1(uint32_t proceso_id);

void liberar_marco(uint32_t nro_marco);

void destruir_Entrada_tabla_nvl2(Entrada_tabla_nvl2* fila);
void destruir_tablas_nvl2(Entrada_tabla_nvl1* tabla_nvl2);
void destruir_tablas_nvl1(Tabla_nvl1* tabla_nvl1);
void destruir_marco(Marco* marco);
void leer_tabla();
void leer_tabla_aux();

/*@desc:Recibe un nodo, modifica sus valores y lo retorna*/
Entrada_tabla_nvl2* modificar_nodo(Entrada_tabla_nvl2*nodo,uint32_t nro_marco,uint32_t nro_pagina,uint32_t bit_uso,uint32_t bit_presencia,uint32_t bit_modificado,uint32_t puntero);

void destruir_list_aux(Remplazo_paginas* nodo);
/*Inicializa una lista con todos los marcos disponibles que tiene la memoria*/
void iniciar_tabla_marcos_disponibles();
#endif /* GLOBALS_H_ */
