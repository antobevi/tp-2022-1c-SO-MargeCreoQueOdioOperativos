/*
 * test.h
 *
 *  Created on: 6 jun. 2022
 *      Author: utnso
 */

#ifndef TEST_H_
#define TEST_H_

#include "memoria.h"
#include "globals.h"

void correr_test();

/*Creo procesos en memoria de manera manual*/
void iniciar_procesos_en_memoria();

//Verifico que los procesos esten en sus respectivas tablas
void verificar_procesos_tabla();

#endif /* TEST_H_ */
