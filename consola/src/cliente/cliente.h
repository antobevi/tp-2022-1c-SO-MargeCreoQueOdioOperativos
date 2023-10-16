#ifndef CLIENTE_CLIENTE_H_
#define CLIENTE_CLIENTE_H_

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <commons/config.h>

#define SERVER_NAME_LEN_MAX 255

// Prototipos de funciones

void mandar_credencial_al_patova(int);
int conectar_con_servidor(char*, int);
void liberar_conexion(int);

#endif /* CLIENTE_CLIENTE_H_ */
