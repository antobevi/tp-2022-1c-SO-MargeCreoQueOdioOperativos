/*
 * conexion.h
 *
 *  Created on: 16 may. 2022
 *      Author: utnso
 */

#ifndef AUXILIARES_CONEXION_H_
#define AUXILIARES_CONEXION_H_
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

void mandar_credencial(int socket);
int conectar_con_memoria(char* ip, int puerto);

#endif /* AUXILIARES_CONEXION_H_ */
