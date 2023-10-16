#ifndef SERVIDOR_SERVIDOR_H_
#define SERVIDOR_SERVIDOR_H_

#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "../planificacion.h" // Ya incluye globals.h y paquetes.h, y paquetes.h incluye cliente.h

// Variables

t_log* log_servidor;
t_config* config;
pthread_mutex_t sem_log_servidor;
bool server_levantado;
int socket_servidor;

// Prototipos de funciones

int levantar_servidor();
void cerrar_servidor();
void atender_cliente(int);
bool patova(int32_t);

void* deserializar(int, void*, int);
void ejecutar_operacion(void*, int, int);

#endif /* SERVIDOR_SERVIDOR_H_ */
