/*
 * swap.h
 *
 *  Created on: 11 jul. 2022
 *      Author: utnso
 */

#ifndef SWAP_H_
#define SWAP_H_

#include <string.h>
#include "globals.h"
#include <stdio.h>
#include <errno.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include  <string.h>
#include "malloc.h"
#include <fcntl.h>
#include <sys/mman.h>

void crear_archivos(char* archivo_swap,int tam_archivo);
void escribir_en_swap(char* nombre_archivo,uint32_t tamanio_swap, uint32_t pagina,void* contenido_pagina);
void* lectura_de_swap(char* nombre_archivo ,uint32_t dir, uint32_t tamanio);
void borrar_archivo(char* archivo_swap);

#endif /* SWAP_H_ */
